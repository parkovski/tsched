#include "bedviewdlg.h"
#include "bedview.h"
#include "truss.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/event.h>

BedViewDlg::BedViewDlg(wxWindow *parent, Truss *truss)
    : wxDialog(parent, wxID_ANY, wxT("Truck Bed View"),
        wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX) {

    wxSizer *btnSizer = CreateButtonSizer(wxOK | wxCANCEL);
    
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *line1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *line1_left = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *line1_right = new wxBoxSizer(wxVERTICAL);
    
    //-----
    line1_left->Add(new wxStaticText(this, wxID_ANY, wxT("Job title:")),
        wxSizerFlags().Border());
    title = new wxTextCtrl(this, wxID_ANY, truss->gettext());
    line1_right->Add(title, wxSizerFlags(1).Expand().Border());
    
    line1_left->Add(new wxStaticText(this, wxID_ANY, wxT("Notes:")),
        wxSizerFlags().Border());
	// On Windows, this control messes up the window size for some reason if it
	// has many lines of text in it, so instead we wait and set the text later.
    notes = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    line1_right->Add(notes, wxSizerFlags(2).Expand().Border());
    
    line1->Add(line1_left, wxSizerFlags());
    line1->Add(line1_right, wxSizerFlags(1).Expand());
    
    //-----
    wxStaticBoxSizer *extInfoSizer =
        new wxStaticBoxSizer(wxVERTICAL, this, wxT("Extended information"));
    
    full_truck = new wxCheckBox(this, wxID_ANY, wxT("Full truck"));
    oversize = new wxCheckBox(this, wxID_ANY, wxT("Oversize load"));
    very_oversize = new wxCheckBox(this, wxID_ANY,
        wxT("Oversize load - special permit required"));
        
    if (truss->has(Truss::kFull))
        full_truck->SetValue(true);
    if (truss->has(Truss::kOversize))
        oversize->SetValue(true);
    if (truss->has(Truss::kVeryOversize))
        very_oversize->SetValue(true);
    
    extInfoSizer->Add(full_truck, wxSizerFlags(1).Border());
    extInfoSizer->Add(oversize, wxSizerFlags(1).Border());
    extInfoSizer->Add(very_oversize, wxSizerFlags(1).Border());
    
    //-----
    // Make total borders of 10 around the dialog. Some of the sizers already
    // have borders.
    sizer->Add(line1, wxSizerFlags().Expand().Border());
    sizer->Add(extInfoSizer, wxSizerFlags().Expand().Border());
    bedview = new BedView(this, truss);
    sizer->Add(bedview, wxSizerFlags(1).Expand().Border());
    sizer->Add(btnSizer, wxSizerFlags().Expand().Border());
    
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &BedViewDlg::onCheckBoxClicked, this);
    
    SetSizerAndFit(sizer);
	
	// See above note.
	notes->SetValue(truss->getnotes());
	
    Centre(wxBOTH);
}

wxString
BedViewDlg::getTitleText() const {
    return title->GetValue();
}

wxString
BedViewDlg::getNotesText() const {
    return notes->GetValue();
}

unsigned
BedViewDlg::getTrussFlags() const {
    unsigned flags = 0;
    if (full_truck->GetValue())
        flags |= Truss::kFull;
    if (oversize->GetValue())
        flags |= Truss::kOversize;
    if (very_oversize->GetValue())
        flags |= Truss::kVeryOversize;
    
    return flags;
}

void
BedViewDlg::onCheckBoxClicked(wxCommandEvent &event) {
    unsigned flags = 0;
    
    // Very oversize always implies oversize. Likewise, not oversize implies
    // not very oversize.
    if (event.GetEventObject() == oversize && !oversize->GetValue())
        very_oversize->SetValue(false);
    else if (event.GetEventObject() == very_oversize
        && very_oversize->GetValue()) {
        
        oversize->SetValue(true);
    }
    
    if (full_truck->GetValue())
        flags |= Truss::kFull;
    if (oversize->GetValue())
        flags |= Truss::kOversize;
    if (very_oversize->GetValue())
        flags |= Truss::kVeryOversize;
    
    bedview->setFlags(flags);
    
    event.Skip();
}
