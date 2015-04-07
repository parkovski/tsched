#ifndef treatasdlg_h
#define treatasdlg_h

#include "trereader.h"

#include <wx/dialog.h>

class wxRadioBox;

class TreatAsDlg : public wxDialog {
    wxRadioBox *radiobox;

public:
    TreatAsDlg(wxWindow *parent, const std::string &truss_type);
    
    TREReader::truss_type getSelection() const;
};

#endif // treatasdlg_h
