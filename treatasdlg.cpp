#include "treatasdlg.h"

#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <cassert>

TreatAsDlg::TreatAsDlg(wxWindow *parent, const std::string &truss_type)
    : wxDialog(parent, wxID_ANY, wxT("Import truss as")) {
    
    wxSizer *btnSizer = CreateButtonSizer(wxOK | wxCANCEL);
    
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    
    const wxString choices[] = {
        wxT("Common"),
        wxT("Sloping Flat"),
        wxT("Floor")
    };
    radiobox = new wxRadioBox(this, wxID_ANY, wxT("Truss type"),
        wxDefaultPosition, wxDefaultSize, sizeof(choices)/sizeof(choices[0]),
        choices);
    radiobox->SetSelection(0);
    
    sizer->Add(new wxStaticText(this, wxID_ANY,
        wxT("The truss type \"") + truss_type +
            wxT("\" is not recognized.\nChoose a type to import it as:")),
        wxSizerFlags().Expand().Border());
    sizer->Add(radiobox, wxSizerFlags().Expand().Border());
    sizer->Add(btnSizer, wxSizerFlags().Expand().Border());
    
    SetSizerAndFit(sizer);
    Centre(wxBOTH);
}

TREReader::truss_type
TreatAsDlg::getSelection() const {
    // This assumes that the indexes from the radiobox line up with truss_type.
    assert(0 == TREReader::tt_invalid && 3 == TREReader::tt_floor &&
        "this function will return invalid results");
    
    return (TREReader::truss_type)(radiobox->GetSelection() + 1);
}
