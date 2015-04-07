#include "daypickerdlg.h"

#include <wx/event.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

DayPickerDlg::DayPickerDlg(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, wxT("Pick a day")), day(-1) {
    
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    
    buttons->Add(new wxButton(this, 1, wxT("Monday")), wxSizerFlags());
    buttons->Add(new wxButton(this, 2, wxT("Tuesday")), wxSizerFlags());
    buttons->Add(new wxButton(this, 3, wxT("Wednesday")), wxSizerFlags());
    buttons->Add(new wxButton(this, 4, wxT("Thursday")), wxSizerFlags());
    buttons->Add(new wxButton(this, 5, wxT("Friday")), wxSizerFlags());
    buttons->Add(new wxButton(this, 6, wxT("Saturday")), wxSizerFlags());
    
    sizer->Add(new wxStaticText(this, wxID_ANY,
        wxT("Select a day to insert the truss into:")),
        wxSizerFlags().Expand().Border());
    sizer->Add(buttons, wxSizerFlags().Expand().Border());
    
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DayPickerDlg::onButtonPressed, this);
    
    SetSizerAndFit(sizer);
    Centre(wxBOTH);
}

void DayPickerDlg::onButtonPressed(wxCommandEvent &event) {
    day = event.GetId()-1;
    EndModal(wxID_OK);
}
