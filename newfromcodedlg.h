#ifndef newfromcodedlg_h
#define newfromcodedlg_h

#include <wx/dialog.h>

class wxTextCtrl;
class wxChoice;

class NewFromCodeDlg : public wxDialog {
    wxTextCtrl *codectrl;
    wxChoice *dayctrl;
    
public:
    NewFromCodeDlg(wxWindow *parent);
    
    // returns -1 for invalid
    int getDay() const;
    wxString getCode() const;
};

#endif // newfromcodedlg_h
