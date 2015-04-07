#ifndef bedviewdlg_h
#define bedviewdlg_h

#include <wx/dialog.h>

class wxTextCtrl;
class wxCheckBox;
class wxCommandEvent;
class Truss;
class BedView;

class BedViewDlg : public wxDialog {
    wxTextCtrl *title;
    wxTextCtrl *notes;
    wxCheckBox *full_truck;
    wxCheckBox *oversize;
    wxCheckBox *very_oversize;
    BedView *bedview;
    
public:
    BedViewDlg(wxWindow *parent, Truss *truss);
        
    wxString getTitleText() const;
    wxString getNotesText() const;
    
    unsigned getTrussFlags() const;
    
    void onCheckBoxClicked(wxCommandEvent &event);
};

#endif // bedviewdlg_h
