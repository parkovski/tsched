#ifndef daypickerdlg_h
#define daypickerdlg_h

#include <wx/dialog.h>

class wxCommandEvent;

class DayPickerDlg : public wxDialog {
    int day; // 0 = monday, 5 = saturday, -1 = cancelled.
    
public:
    DayPickerDlg(wxWindow *parent);
    
    void onButtonPressed(wxCommandEvent &event);
    
    int getDay() const { return day; }
};

#endif // daypickerdlg_h
