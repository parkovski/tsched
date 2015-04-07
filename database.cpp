#include "database.h"
#include "truss.h"

#include <wx/config.h>

#include <sqlite3.h>

#include <cstdio>

// Returns an empty string if the database filename couldn't be found.
wxString db_filename() {
    wxConfig config(wxT("TrussScheduler"));
    
    wxString dbname;
    if (config.Read(wxT("/Settings/DbFilename"), &dbname))
        return dbname;

    return wxT("");
}

// Returns true if the name was successfully set, otherwise false.
bool db_set_filename(const wxString &new_filename) {
    wxConfig config(wxT("TrussScheduler"));

    return config.Write(wxT("/Settings/DbFilename"), new_filename);
}

// Creates a database, and registers this filename if successful.
bool db_create(const wxString &filename) {
    bool result = true;
    sqlite3 *db;

    if (sqlite3_open(filename, &db) != SQLITE_OK) {
        result = false;
    } else {
        db_set_filename(filename);
        if (sqlite3_exec(db, "CREATE TABLE trusses(id INTEGER PRIMARY KEY, "
                "customer TEXT, site TEXT, plan TEXT, data TEXT)", 0, 0, 0)
                != SQLITE_OK)
            result = false;
    }

    sqlite3_close(db);

    return result;
}

bool db_exists() {
    return !db_filename().empty();
}

DBConn::DBConn(const char *filename) {
    if (sqlite3_open(filename, &db)) {
        sqlite3_close(db);
        db = 0;
    }
}

DBConn::DBConn() {
    if (sqlite3_open(db_filename(), &db)) {
        sqlite3_close(db);
        db = 0;
    }
}

DBConn::~DBConn() {
    if (db)
        sqlite3_close(db);
}

bool
DBConn::good() const {
    return db != 0;
}

//-----

DBRead::DBRead(const char *filename) : DBConn(filename) {
}

DBRead::DBRead() : DBConn() {
}

DBRead::~DBRead() {
}

int populate_arrstr(void *obj, int argc, char *argv[], char *azColName[]) {
    wxArrayString *arrstr = static_cast<wxArrayString *>(obj);
    if (argv[0])
        arrstr->Add(argv[0]);
    return 0;
}

const wxArrayString
DBRead::getCustomers() const {
    wxArrayString arrstr;
    // If this fails, we will just return an empty list.
    sqlite3_exec(db, "SELECT DISTINCT customer FROM trusses "
            "ORDER BY customer ASC", populate_arrstr, &arrstr, 0);
    return arrstr;
}

const wxArrayString
DBRead::getSites(const char *cust) const {
    wxArrayString arrstr;
    sqlite3_stmt *stmt = 0;
    if (sqlite3_prepare_v2(db,
            "SELECT DISTINCT site FROM trusses WHERE customer=? "
            "ORDER BY site ASC",
            -1, &stmt, 0) == SQLITE_OK) {
        
        if (sqlite3_bind_text(stmt, 1, cust, strlen(cust), SQLITE_TRANSIENT) ==
                SQLITE_OK) {
            
            int step_result;
            do {
                step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_ROW) {
                    arrstr.Add(sqlite3_column_text(stmt, 0));
                } else if (step_result == SQLITE_ERROR ||
                        step_result == SQLITE_BUSY) {
                    break;
                }
            } while (step_result != SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);
    return arrstr;
}

const wxArrayString
DBRead::getPlans(const char *cust, const char *site) const {
    wxArrayString arrstr;
    sqlite3_stmt *stmt = 0;
    if (sqlite3_prepare_v2(db,
            "SELECT DISTINCT plan FROM trusses WHERE customer=? "
            "AND site=? ORDER BY plan ASC",
            -1, &stmt, 0) == SQLITE_OK) {
        
        if (sqlite3_bind_text(stmt, 1, cust, strlen(cust), SQLITE_TRANSIENT) ==
                SQLITE_OK &&
                sqlite3_bind_text(stmt, 2, site, strlen(site),
                    SQLITE_TRANSIENT) == SQLITE_OK
                ) {
            
            int step_result;
            do {
                step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_ROW) {
                    arrstr.Add(sqlite3_column_text(stmt, 0));
                } else if (step_result == SQLITE_ERROR ||
                        step_result == SQLITE_BUSY) {
                    break;
                }
            } while (step_result != SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);
    return arrstr;
}

Truss *
DBRead::getTruss(const char *cust, const char *site, const char *plan) const {
    Truss *truss = 0;
    sqlite3_stmt *stmt = 0;
    if (sqlite3_prepare_v2(db,
            "SELECT data FROM trusses WHERE customer=? "
            "AND site=? AND plan=?",
            -1, &stmt, 0) == SQLITE_OK) {
        
        if (sqlite3_bind_text(stmt, 1, cust, strlen(cust), SQLITE_TRANSIENT) ==
                SQLITE_OK &&
                sqlite3_bind_text(stmt, 2, site, strlen(site),
                    SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 3, plan, strlen(plan),
                    SQLITE_TRANSIENT) == SQLITE_OK
                ) {
            
            int step_result;
            do {
                step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_ROW) {
                    // This should only happen once. A given site, plan,
                    // and customer should result in only one truss.
                    truss = Truss::unpack(
                        (const char *)sqlite3_column_text(stmt, 0));
                } else if (step_result == SQLITE_ERROR ||
                        step_result == SQLITE_BUSY) {
                    break;
                }
            } while (step_result != SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);
    return truss;
}

//-----

DBWrite::DBWrite(const char *filename) : DBConn(filename), lockfilename() {
    // If we can read from the lock file, it means someone else is using the
    // database.
    lockfilename = filename;
    lockfilename += ".lock";
    std::ifstream lockin(lockfilename.c_str());
    if (lockin.good())
        return;

    // Otherwise, we should create the lock file so others know we're using
    // the database.
    lockfile.open(lockfilename.c_str());
}

DBWrite::DBWrite() : DBConn(), lockfilename(db_filename() + ".lock") {
    std::ifstream lockin(lockfilename.c_str());
    if (lockin.good())
        return;

#if defined(_MSC_VER)
	lockfile.open((decltype(wxT("")))(db_filename() + ".lock"));
#else
    lockfile.open(db_filename() + ".lock");
#endif
}

DBWrite::~DBWrite() {
    if (hasLock()) {
        lockfile.close();
        ::remove(lockfilename.c_str());
    }
}

bool
DBWrite::hasLock() const {
    return lockfile.is_open();
}

void
DBWrite::removeLock() {
    if (lockfile.is_open())
        lockfile.close();
    ::remove(lockfilename.c_str());
}

bool
DBWrite::hasTruss(const char *cust, const char *site, const char *plan) const {
    sqlite3_stmt *stmt = 0;
    bool has_truss = false;

    if (sqlite3_prepare_v2(db,
            "SELECT data FROM trusses WHERE customer=? "
            "AND site=? AND plan=?",
            -1, &stmt, 0) == SQLITE_OK) {
        
        if (sqlite3_bind_text(stmt, 1, cust, strlen(cust), SQLITE_TRANSIENT) ==
                SQLITE_OK &&
                sqlite3_bind_text(stmt, 2, site, strlen(site),
                    SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 3, plan, strlen(plan),
                    SQLITE_TRANSIENT) == SQLITE_OK
                ) {
            
            int step_result;
            do {
                step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_ROW) {
                    has_truss = true;
                    break;
                } else if (step_result == SQLITE_ERROR ||
                        step_result == SQLITE_BUSY) {
                    break;
                }
            } while (step_result != SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);
    return has_truss;
}

bool
DBWrite::insert(const char *cust, const char *site, const char *plan,
        Truss *truss) {

    sqlite3_stmt *stmt = 0;
    bool success = true;
    const wxString &truss_data = truss->packStripped();
    const char *data = truss_data.c_str();

    if (sqlite3_prepare_v2(db,
            "INSERT INTO trusses (customer, site, plan, data) "
            "VALUES (?, ?, ?, ?)",
            -1, &stmt, 0) == SQLITE_OK) {

        if (sqlite3_bind_text(stmt, 1, cust, strlen(cust),
                SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 2, site, strlen(site),
                    SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 3, plan, strlen(plan),
                    SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 4, data, strlen(data),
                    SQLITE_TRANSIENT) == SQLITE_OK
            ) {
            
            int step_result;
            do {
                step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_ERROR || step_result == SQLITE_BUSY) {
                    success = false;
                    break;
                }
            } while (step_result != SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);
    return success;
}

bool
DBWrite::remove(const char *cust, const char *site, const char *plan) {
    sqlite3_stmt *stmt = 0;
    bool success = true;

    if (sqlite3_prepare_v2(db,
            "DELETE FROM trusses WHERE customer=? AND site=? AND plan=?",
            -1, &stmt, 0) == SQLITE_OK) {

        if (sqlite3_bind_text(stmt, 1, cust, strlen(cust),
                SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 2, site, strlen(site),
                    SQLITE_TRANSIENT) == SQLITE_OK &&
                sqlite3_bind_text(stmt, 3, plan, strlen(plan),
                    SQLITE_TRANSIENT) == SQLITE_OK
            ) {
            
            int step_result;
            do {
                step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_ERROR || step_result == SQLITE_BUSY) {
                    success = false;
                    break;
                }
            } while (step_result != SQLITE_DONE);
        }
    }
    sqlite3_finalize(stmt);
    return success;
}

bool
DBWrite::update(const char *cust, const char *site, const char *plan,
        const char *newcust, const char *newsite, const char *newplan) {

    std::string sql = "UPDATE trusses SET ";
    bool needs_comma = false;
    sqlite3_stmt *stmt;

    if (newcust) {
        sql += "customer=?";
        needs_comma = true;
    }

    if (newsite) {
        if (needs_comma)
            sql += ", site=?";
        else
            sql += "site=?";
        needs_comma = true;
    }

    if (newplan) {
        if (needs_comma)
            sql += ", plan=?";
        else
            sql += "plan=?";
    }

    sql += " WHERE customer=? AND site=? AND plan=?";

    // success will be set to true if we make it through all the sqlite code
    // alive.
    bool success = false;
    int q_index = 1;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
        goto exit_function;

    if (newcust) {
        if (sqlite3_bind_text(stmt, q_index++, newcust, strlen(newcust),
                SQLITE_TRANSIENT) != SQLITE_OK)
            goto exit_function;
    }

    if (newsite) {
        if (sqlite3_bind_text(stmt, q_index++, newsite, strlen(newsite),
                SQLITE_TRANSIENT) != SQLITE_OK)
            goto exit_function;
    }

    if (newplan) {
        if (sqlite3_bind_text(stmt, q_index++, newplan, strlen(newplan),
                SQLITE_TRANSIENT) != SQLITE_OK)
            goto exit_function;
    }

    if (sqlite3_bind_text(stmt, q_index++, cust, strlen(cust),
            SQLITE_TRANSIENT) != SQLITE_OK)
        goto exit_function;

    if (sqlite3_bind_text(stmt, q_index++, site, strlen(site),
            SQLITE_TRANSIENT) != SQLITE_OK)
        goto exit_function;

    if (sqlite3_bind_text(stmt, q_index++, plan, strlen(plan),
            SQLITE_TRANSIENT) != SQLITE_OK)
        goto exit_function;

    int step_result;
    do {
        step_result = sqlite3_step(stmt);
        if (step_result == SQLITE_ERROR || step_result == SQLITE_BUSY)
            goto exit_function;
    } while (step_result != SQLITE_DONE);

    success = true;

exit_function:
    sqlite3_finalize(stmt);
    return success;
}

