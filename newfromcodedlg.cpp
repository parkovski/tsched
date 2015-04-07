#include "newfromcodedlg.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>

NewFromCodeDlg::NewFromCodeDlg(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, wxT("New truss from code")) {
    
    wxSizer *btnSizer = CreateButtonSizer(wxOK | wxCANCEL);
    
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *line1 = new wxBoxSizer(wxHORIZONTAL);
    
    // for the choice control
    const wxString days_of_week[] = {
        wxT("Monday"),
        wxT("Tuesday"),
        wxT("Wednesday"),
        wxT("Thursday"),
        wxT("Friday"),
        wxT("Saturday")
    };
    
    //-----
    line1->Add(new wxStaticText(this, wxID_ANY, wxT("Code:")),
        wxSizerFlags().Border());
    codectrl = new wxTextCtrl(this, wxID_ANY, wxT(""));
    line1->Add(codectrl, wxSizerFlags(1).Expand().Border());
    
    dayctrl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        6, days_of_week);
    sizer->Add(dayctrl, wxSizerFlags().Expand().Border());
    sizer->Add(line1, wxSizerFlags().Expand());
    sizer->Add(btnSizer, wxSizerFlags().Expand().Border());
    
    SetSizerAndFit(sizer);
    Centre(wxBOTH);
}

int NewFromCodeDlg::getDay() const {
    int day = dayctrl->GetSelection();
    if (day == wxNOT_FOUND)
        return -1;
    return day;
}

wxString NewFromCodeDlg::getCode() const {
    return codectrl->GetValue();
}
