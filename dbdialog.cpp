#include "dbdialog.h"
#include "database.h"
#include "bedview.h"

#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/event.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>

DbDialog::DbDialog(wxWindow *parent, bool manage)
    : wxDialog(parent, wxID_ANY, wxT("Database"),
        wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX) {

    // If we're managing trusses, the dialogs handle everything, so there is
    // no need for a "Cancel" button.
    long btnflags = wxOK;
    if (!manage)
        btnflags |= wxCANCEL;
    wxSizer *btnSizer = CreateButtonSizer(btnflags);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *topline = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *column1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *column2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *column3 = new wxBoxSizer(wxVERTICAL);

    DBRead dbread;
    if (!dbread.good()) {
        wxMessageBox(wxT("Couldn't connect to the database! ")
            wxT("Perhaps there is no connection with the server, or the database ")
            wxT("doesn't exist yet. (In this case, click the Database menu, ")
            wxT("and then Setup...)"), wxT("Error"));
    }

    column1->Add(new wxStaticText(this, wxID_ANY, wxT("Customer:")),
        wxSizerFlags().Border());
    customer = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, 0, 0, 0);
    customer->Set(dbread.getCustomers());
    column1->Add(customer, wxSizerFlags(1).Expand().Border());

    column2->Add(new wxStaticText(this, wxID_ANY, wxT("Site:")),
        wxSizerFlags().Border());
    site = new wxComboBox(this, wxID_ANY);
    column2->Add(site, wxSizerFlags(1).Expand().Border());

    column3->Add(new wxStaticText(this, wxID_ANY, wxT("Plan:")),
        wxSizerFlags().Border());
    plan = new wxComboBox(this, wxID_ANY);
    column3->Add(plan, wxSizerFlags(1).Expand().Border());

    topline->Add(column1, wxSizerFlags(1).Expand().Border());
    topline->Add(column2, wxSizerFlags(1).Expand().Border());
    topline->Add(column3, wxSizerFlags(1).Expand().Border());

    if (manage) {
        wxBoxSizer *column4 = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *column5 = new wxBoxSizer(wxVERTICAL);

        wxButton *btnmove, *btndel;
        // Use an empty static text to make sure the buttons align with the
        // combo boxes on the same line. (Use a non breaking space).
        column4->Add(new wxStaticText(this, wxID_ANY, wxT("\u00A0")),
            wxSizerFlags().Border());
        column4->Add(btnmove = new wxButton(this, wxID_ANY, wxT("Move...")),
            wxSizerFlags().Border());

        column5->Add(new wxStaticText(this, wxID_ANY, wxT("\u00A0")),
            wxSizerFlags().Border());
        column5->Add(btndel = new wxButton(this, wxID_ANY, wxT("Delete")),
            wxSizerFlags().Border());

        topline->Add(column4, wxSizerFlags().Border());
        topline->Add(column5, wxSizerFlags().Border());

        btnmove->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
            &DbDialog::onMovePressed, this);
        btndel->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
            &DbDialog::onDeletePressed, this);
    }

    bedview = new BedView(this, 0);

    sizer->Add(topline, wxSizerFlags().Expand().Border());
    sizer->Add(bedview, wxSizerFlags(1).Expand().Border());
    sizer->Add(btnSizer, wxSizerFlags().Expand().Border());

    Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &DbDialog::onComboBoxChanged, this);
    Bind(wxEVT_COMMAND_TEXT_ENTER, &DbDialog::onComboBoxChanged, this);

    SetSizerAndFit(sizer);

    Centre(wxBOTH);
}

void
DbDialog::onComboBoxChanged(wxCommandEvent &event) {
    if (event.GetEventObject() == customer) {
        DBRead dbread;
        if (!dbread.good())
            return;

        site->SetValue(wxT(""));
        site->Set(dbread.getSites(customer->GetValue()));
        plan->SetValue(wxT(""));
        plan->Clear();
        bedview->setTruss(0, false);
    } else if (event.GetEventObject() == site) {
        DBRead dbread;
        if (!dbread.good())
            return;

        const wxArrayString &plans = dbread.getPlans(customer->GetValue(),
            site->GetValue());
        plan->Set(plans);
        if (plans.size() == 1) {
            plan->SetSelection(0);
            bedview->setTruss(dbread.getTruss(customer->GetValue(),
                site->GetValue(), plan->GetValue()), true);
        } else {
            plan->SetValue(wxT(""));
            bedview->setTruss(0, false);
        }
    } else if (event.GetEventObject() == plan) {
        DBRead dbread;
        if (!dbread.good())
            return;

        Truss *truss = dbread.getTruss(customer->GetValue(), site->GetValue(),
            plan->GetValue());
        bedview->setTruss(truss, truss != 0);
    }
}

void
DbDialog::onMovePressed(wxCommandEvent &event) {
    DbRenameDlg rdlg(this,
        customer->GetValue(), site->GetValue(), plan->GetValue());
    if (wxID_CANCEL == rdlg.ShowModal())
        return;

    // See below for reasoning for the scope block.
    {
        DBWrite dbwrite;
        if (!dbwrite.good()) {
            wxMessageBox(wxT("Couldn't open the database!"), wxT("Error"));
            return;
        }

        // If we don't have the lock, there's a chance that the file could be
        // there erroneously, so just prompt the user.
        if (!dbwrite.hasLock()) {
            if (wxNO == wxMessageBox(wxT("I couldn't acquire the lock file. ")
                    wxT("If you proceed, you risk corrupting the database, ")
                    wxT("but if no one is using it, it could be a mistake and you ")
                    wxT("will be fine. Do you want to proceed?"),
                    wxT("Confirm"), wxYES_NO)) {

                return;
            }
            dbwrite.removeLock();
        }

        if (dbwrite.hasTruss(rdlg.getNewCust(), rdlg.getNewSite(),
            rdlg.getNewPlan())) {

            wxMessageBox(wxT("This information already identifies a truss. ")
                wxT("If you really want to overwrite it, please delete it first."),
                wxT("Cannot overwrite truss"));

            return;
        }

        if (!dbwrite.update(customer->GetValue(), site->GetValue(),
                plan->GetValue(),
            rdlg.getNewCust(), rdlg.getNewSite(), rdlg.getNewPlan())) {

            wxMessageBox(wxT("The truss couldn't be updated in the database."),
                wxT("Database error"));

            return;
        }
    }

    // This will update the combo boxes' list, and make sure they aren't
    // showing something that doesn't exist anymore. Since this truss still
    // exists, just under a different name, we need to update that.
    updateAfterChange(true /* keep the truss */);
    customer->SetValue(rdlg.getNewCust());
    site->SetValue(rdlg.getNewSite());
    plan->SetValue(rdlg.getNewPlan());

    wxMessageBox(wxT("You have successfully moved the truss. The new customer, ")
        wxT("site, and plan values are now visible in the drop-down boxes."),
        wxT("Alert"));
}

void
DbDialog::onDeletePressed(wxCommandEvent &event) {
    int result = wxMessageBox(
        wxT("Are you sure you want to delete this truss from the database? ")
            wxT("This operation cannot be undone."),
        wxT("Confirm operation"),
        wxYES_NO
    );

    if (result == wxNO)
        return;
    
    // Keep the dbwrite in its own scope block, so that it is closed by the
    // time the reader needs to update the UI.
    {
        DBWrite dbwrite;
        if (dbwrite.good() && !dbwrite.hasLock()) {
            if (!dbwrite.hasLock()) {
                if (wxNO == wxMessageBox(
                        wxT("I couldn't acquire the lock file. ")
                        wxT("If you proceed, you risk corrupting the database, ")
                        wxT("but if no one is using it, it could be a mistake and ")
                        wxT("you will be fine. Do you want to proceed?"),
                        wxT("Confirm"), wxYES_NO)) {

                    return;
                }
                dbwrite.removeLock();
            }
        }

        if (!dbwrite.good() || !dbwrite.remove(customer->GetValue(),
                site->GetValue(), plan->GetValue())) {

            wxMessageBox(wxT("The truss couldn't be deleted."), wxT("Sorry"));
        }
    }

    updateAfterChange(false);

    wxMessageBox(wxT("The truss was successfully removed from the database."),
        wxT("Alert"));
}

void
DbDialog::updateAfterChange(bool keepTruss) {
    DBRead dbread;

    if (!keepTruss)
        bedview->setTruss(0, false);

    if (!dbread.good()) {
        customer->Clear();
        site->Clear();
        plan->Clear();
        customer->SetValue(wxT(""));
        site->SetValue(wxT(""));
        plan->SetValue(wxT(""));
    }

    const wxString &oldcust = customer->GetValue();
    wxArrayString customers = dbread.getCustomers();
    customer->Set(customers);
    if (wxNOT_FOUND != customers.Index(oldcust)) {
        customer->SetValue(oldcust);
    } else {
        customer->SetValue(wxT(""));
        site->Clear();
        plan->Clear();
        site->SetValue(wxT(""));
        plan->SetValue(wxT(""));
        return;
    }

    const wxString &oldsite = site->GetValue();
    wxArrayString sites = dbread.getSites(oldcust);
    site->Set(sites);
    if (wxNOT_FOUND != sites.Index(oldsite)) {
        site->SetValue(oldsite);
    } else {
        plan->Clear();
        site->SetValue(wxT(""));
        plan->SetValue(wxT(""));
        return;
    }

    // I think it's safe to assume that at least the plan will have changed.
    plan->Set(dbread.getPlans(oldcust, oldsite));
    plan->SetValue(wxT(""));
}

Truss *
DbDialog::getTruss() const {
    DBRead dbread;
    return dbread.getTruss(customer->GetValue(), site->GetValue(),
        plan->GetValue());
}

//-----
// DbRenameDlg
//-----

DbRenameDlg::DbRenameDlg(wxWindow *parent, const wxString &oldcust,
    const wxString &oldsite, const wxString &oldplan)
        : wxDialog(parent, wxID_ANY, wxT("Move/Rename Truss")),
        oldcust(oldcust), oldsite(oldsite), oldplan(oldplan) {

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *line1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *line2 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *line3 = new wxBoxSizer(wxHORIZONTAL);

    wxSizer *buttons = CreateButtonSizer(wxOK | wxCANCEL);

    DBRead dbread;

    line1->Add(new wxStaticText(this, wxID_ANY, wxT("Customer:")),
        wxSizerFlags(1).Border().Align(wxALIGN_CENTER_VERTICAL));
    newcust = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, 0, 0, 0);
    newcust->Set(dbread.getCustomers());
    line1->Add(newcust, wxSizerFlags(4).Expand().Border());

    line2->Add(new wxStaticText(this, wxID_ANY, wxT("Site:")),
        wxSizerFlags(1).Border().Align(wxALIGN_CENTER_VERTICAL));
    newsite = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, 0, 0, 0);
    line2->Add(newsite, wxSizerFlags(4).Expand().Border());

    line3->Add(new wxStaticText(this, wxID_ANY, wxT("Plan:")),
        wxSizerFlags(1).Border().Align(wxALIGN_CENTER_VERTICAL));
    newplan = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, 0, 0, 0);
    line3->Add(newplan, wxSizerFlags(4).Expand().Border());

    if (!oldcust.empty()) {
        newsite->Set(dbread.getSites(oldcust));
        newplan->Set(dbread.getPlans(oldcust, oldsite));

        newcust->SetValue(oldcust);
        newsite->SetValue(oldsite);
        newplan->SetValue(oldplan);
    }

    sizer->Add(line1, wxSizerFlags().Expand().Border());
    sizer->Add(line2, wxSizerFlags().Expand().Border());
    sizer->Add(line3, wxSizerFlags().Expand().Border());
    sizer->Add(buttons, wxSizerFlags().Expand().Border());

    Bind(wxEVT_COMMAND_COMBOBOX_SELECTED,
        &DbRenameDlg::onComboBoxChanged, this);

    SetSizerAndFit(sizer);

    Centre(wxBOTH);
}

void
DbRenameDlg::onComboBoxChanged(wxCommandEvent &event) {
    if (event.GetEventObject() == newcust) {
        DBRead dbread;
        newsite->SetValue(wxT(""));
        newsite->Set(dbread.getSites(newcust->GetValue()));
        newplan->SetValue(wxT(""));
        newplan->Clear();
    } else if (event.GetEventObject() == newsite) {
        DBRead dbread;
        const wxArrayString &plans = dbread.getPlans(newcust->GetValue(),
            newsite->GetValue());
        newplan->Set(plans);
        if (plans.size() == 1)
            newplan->SetSelection(0);
        else
            newplan->SetValue(wxT(""));
    }
}

wxString
DbRenameDlg::getNewCust() const {
    return newcust->GetValue();
}

wxString
DbRenameDlg::getNewSite() const {
    return newsite->GetValue();
}

wxString
DbRenameDlg::getNewPlan() const {
    return newplan->GetValue();
}

// Tries to import a truss to the database, depending on user input and
// database availability.
bool db_try_import(wxWindow *parent, Truss *truss) {
    DbRenameDlg dlg(parent, wxT(""), wxT(""), wxT(""));

    if (dlg.ShowModal() == wxID_CANCEL)
        return false;

    DBWrite dbwrite;
    if (!dbwrite.good()) {
        wxMessageBox(wxT("Couldn't connect to the database"), wxT("Error!"));
        return false;
    }

    if (!dbwrite.hasLock()) {
        int do_anyway = wxMessageBox(wxT("I couldn't acquire the lock. If you ")
            wxT("proceed, you risk corrupting the database. However, honestly, ")
            wxT("if you took the time to read this, it's probably safe now. ")
            wxT("Your call. (You can always say no and try again later ")
            wxT("just to be safe.)"), wxT("Confirm"), wxYES_NO, parent);

        if (do_anyway == wxNO)
            return false;

        dbwrite.removeLock();
    }

    if (dbwrite.hasTruss(dlg.getNewCust(), dlg.getNewSite(),
            dlg.getNewPlan())) {

        wxMessageBox(wxT("The customer, site, and plan you gave already ")
            wxT("identify a truss. If you really want to replace it, please ")
            wxT("delete the old one first."), wxT("Alert"));

        return false;
    }

    dbwrite.insert(dlg.getNewCust(), dlg.getNewSite(), dlg.getNewPlan(), truss);
	return true;
}

//-----
// DbSetupDlg
//-----

DbSetupDlg::DbSetupDlg(wxWindow *parent)
        : wxDialog(parent, wxID_ANY, wxT("Setup Database"),
            wxDefaultPosition, wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *filebox = new wxBoxSizer(wxHORIZONTAL);

    wxSizer *buttons = CreateButtonSizer(wxOK | wxCANCEL);
    
    sizer->AddStretchSpacer(1);
    radio_have_db = new wxRadioButton(this, wxID_ANY,
        wxT("I already have a database"), wxDefaultPosition, wxDefaultSize,
        wxRB_GROUP);
    sizer->Add(radio_have_db, wxSizerFlags(0).Border());
    radio_need_db = new wxRadioButton(this, wxID_ANY,
        wxT("I need to create a new database"));
    sizer->Add(radio_need_db, wxSizerFlags(0).Border());
    radio_have_db->SetValue(true);
    sizer->AddStretchSpacer(1);
    
    sizer->Add(label = new wxStaticText(this, wxID_ANY,
        wxT("Enter the location of the existing database:")),
        wxSizerFlags().Border());
    filename = new wxTextCtrl(this, wxID_ANY, wxT(""));
    filename->SetMinSize(wxSize(300, 15));
    filename->SetValue(db_filename());
    filebox->Add(filename, wxSizerFlags(1).Expand().Border());
    browse = new wxButton(this, wxID_ANY, wxT("Browse..."));
    filebox->Add(browse, wxSizerFlags(0).Expand().Border());

    sizer->Add(filebox, wxSizerFlags().Expand().Border());
    sizer->AddStretchSpacer(1);
    sizer->Add(buttons, wxSizerFlags().Expand().Border());

    Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &DbSetupDlg::onRadioButton, this);
    browse->Bind(
        wxEVT_COMMAND_BUTTON_CLICKED, &DbSetupDlg::onBrowseButton, this
    );

    SetSizerAndFit(sizer);
}

void
DbSetupDlg::onRadioButton(wxCommandEvent &event) {
    if (event.GetEventObject() == radio_have_db) {
        label->SetLabelText(
            wxT("Enter the location of the existing database:"));
        filename->SetValue(db_filename());
    } else {
        label->SetLabelText(wxT("Enter a location for the new database:"));
        filename->Clear();
    }
}

void
DbSetupDlg::onBrowseButton(wxCommandEvent &event) {
    int flags;
    if (radio_have_db->GetValue())
        flags = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
    else
        flags = wxFD_SAVE | wxFD_OVERWRITE_PROMPT;

    wxFileDialog dlg(this, wxT("Select database location"), wxT(""), wxT(""),
            wxT("Database files|*.db|All files|*.*"), flags);

    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    const wxString &path = dlg.GetPath();
    filename->SetLabelText(path);
}

bool
DbSetupDlg::hasDb() const {
    return radio_have_db->GetValue();
}

wxString
DbSetupDlg::getFilename() const {
    return filename->GetValue();
}

