#ifndef database_h
#define database_h

#include <wx/arrstr.h>

#include <fstream>

struct sqlite3;
class Truss;

wxString db_filename();
bool db_set_filename(const wxString &new_filename);
bool db_create(const wxString &filename);
bool db_exists();

class DBConn {
protected:
    // This is declared as mutable because sqlite doesn't know that the reader's
    // queries never change the database.
    mutable sqlite3 *db;

public:
    DBConn(const char *filename);
    DBConn();
    ~DBConn();

    bool good() const;
};

class DBRead : public DBConn {
public:
    DBRead(const char *filename);
    DBRead();
    ~DBRead();

    const wxArrayString getCustomers() const;
    const wxArrayString getSites(const char *cust) const;
    const wxArrayString getPlans(const char *cust, const char *site) const;
    Truss *getTruss(const char *cust, const char *site, const char *plan) const;
};

class DBWrite : public DBConn {
    std::ofstream lockfile;
    std::string lockfilename;

public:
    DBWrite(const char *filename);
    DBWrite();
    ~DBWrite();

    // If this returns false, somebody else has locked the database, and you
    // shouldn't do anything with it (though if good() returns true, this
    // class will still let you).
    bool hasLock() const;
    void removeLock();

    // We are not allowed to create duplicates, so this lets you check.
    bool hasTruss(const char *cust, const char *site, const char *plan) const;
    bool insert(const char *cust, const char *site, const char *plan,
        Truss *truss);
    bool remove(const char *cust, const char *site, const char *plan);

    // If you specify all three parameters, only one truss will be moved.
    // Otherwise, all trusses in those categories will be renamed/moved.
    // Any non-null new* field will be changed on the trusses you select.
    bool update(const char *cust, const char *site, const char *plan,
        const char *newcust, const char *newsite, const char *newplan);
};

#endif // database_h

