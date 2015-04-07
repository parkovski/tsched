#ifndef dbdialog_h
#define dbdialog_h

#include <wx/dialog.h>

class wxComboBox;
class wxRadioButton;
class wxTextCtrl;
class wxStaticText;
class wxButton;
class BedView;
class Truss;

class DbDialog : public wxDialog {
    wxComboBox *customer;
    wxComboBox *site;
    wxComboBox *plan;
    BedView *bedview;

    void updateAfterChange(bool keepTruss);

public:
    DbDialog(wxWindow *parent, bool manage);

    void onComboBoxChanged(wxCommandEvent &event);
    void onMovePressed(wxCommandEvent &event);
    void onDeletePressed(wxCommandEvent &event);

    Truss *getTruss() const;
};

class DbRenameDlg : public wxDialog {
    wxComboBox *newcust;
    wxComboBox *newsite;
    wxComboBox *newplan;

    wxString oldcust;
    wxString oldsite;
    wxString oldplan;

public:
    // Pass "" for the string parameters if it is not already in the database.
    DbRenameDlg(wxWindow *parent, const wxString &oldcust,
        const wxString &oldsite, const wxString &oldplan);

    void onComboBoxChanged(wxCommandEvent &event);

    wxString getNewCust() const;
    wxString getNewSite() const;
    wxString getNewPlan() const;
};

bool db_try_import(wxWindow *parent, Truss *truss);

class DbSetupDlg : public wxDialog {
    wxRadioButton *radio_have_db;
    wxRadioButton *radio_need_db;
    wxTextCtrl *filename;
    wxStaticText *label;
    wxButton *browse;

public:
    DbSetupDlg(wxWindow *parent);

    void onRadioButton(wxCommandEvent &event);
    void onBrowseButton(wxCommandEvent &event);
    
    bool hasDb() const;
    wxString getFilename() const;
};

#endif // dbdialog_h

