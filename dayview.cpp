#include "dayview.h"
#include "trussthumbview.h"
#include "truss.h"
#include "weekview.h"
#include "frame.h"
#include "printing.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/settings.h>
#include <wx/dnd.h>
#include <wx/event.h>
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>
#include <wx/utils.h>

class DayViewDropTarget : public wxTextDropTarget {
    DayView *dv;
    
public:
    DayViewDropTarget(DayView *dv) : dv(dv) { }
    
    bool OnDropText(wxCoord x, wxCoord y, const wxString &data) {
        Truss *truss = Truss::unpack(data.c_str());
        if (!truss)
            return false;

        TrussThumbView *view = dv->getViewAt(x, y);
        TrussThumbView *newview;
        if (!view)
            newview = dv->addThumbView(truss);
        else
            newview = dv->insertBefore(view, truss);
        
        ((WeekView *)dv->GetParent())->select(newview);
        return true;
    }
};

DayView::DayView(wxWindow *parent, const wxString &text, bool leftBorder,
        int daynr)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize),
        leftBorder(leftBorder), daynr(daynr) {

    sizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticText *label;
    
    sizer->Add(
        label = new wxStaticText(this, wxID_ANY, text,
            wxDefaultPosition, wxDefaultSize,
            wxALIGN_CENTRE | wxST_NO_AUTORESIZE),
        0,
        wxEXPAND | wxALL,
        5
    );
    
    label->Bind(wxEVT_RIGHT_DOWN, &DayView::onRightClick, this);
    
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    SetSizer(sizer);
    
    SetDropTarget(new DayViewDropTarget(this));
    
    Bind(wxEVT_LEFT_DOWN, &DayView::onClick, this);
    Bind(wxEVT_PAINT, &DayView::onPaint, this);
    Bind(wxEVT_RIGHT_DOWN, &DayView::onRightClick, this);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DayView::onMenu, this);
}

DayView::~DayView() {
}

TrussThumbView *
DayView::addThumbView(Truss *truss) {
    TrussThumbView *view = new TrussThumbView(this, truss);
    sizer->Add(view, 0, wxEXPAND | wxALL, 5);
    Layout();
    GetParent()->FitInside();
    return view;
}

void
DayView::removeThumbView(TrussThumbView *view) {
    sizer->Detach(view);
}

bool
DayView::empty() const {
    // The header label will always be there.
    return sizer->GetItemCount() == 1;
}

void
DayView::writeAll(std::ostream &out) {
    int index = 1;
    int max = sizer->GetItemCount();
    
    while (index < max) {
        TrussThumbView *view =
            (TrussThumbView *)sizer->GetItem(index)->GetWindow();
        
        out << view->getTruss()->pack() << "\n";
        ++index;
    }
}

// From app.cpp.
extern wxPrintData *print_data;

void
DayView::printAll() {
    const char * const titles[] = {
        "You shouldn't see this",
        "Jobs for Monday",
        "Jobs for Tuesday",
        "Jobs for Wednesday",
        "Jobs for Thursday",
        "Jobs for Friday",
        "Jobs for Saturday"
    };
    
    int index = 1;
    int max = sizer->GetItemCount();
    if (max == 1)
        return; // If there's nothing don't print anything, obviously.

    TrussPrintout p(titles[daynr+1]);
    
    while (index < max) {
        TrussThumbView *view =
            (TrussThumbView *)sizer->GetItem(index)->GetWindow();
        p.addTruss(view->getTruss());
        ++index;
    }
    
    wxPrintDialogData data(*print_data);
    wxPrinter printer(&data);
    bool success = printer.Print(this, &p, true);
}

void
DayView::addToPrintout(TrussPrintout &p) const {
    int index = 1;
    int max = sizer->GetItemCount();
    if (max == 1)
        return;
    
    while (index < max) {
        TrussThumbView *view =
            (TrussThumbView *)sizer->GetItem(index)->GetWindow();
        p.addTruss(view->getTruss());
        ++index;
    }
}

TrussThumbView *
DayView::getViewAt(wxCoord x, wxCoord y) {
    int last_y = 0;
    int index = 1, nr_items = sizer->GetItemCount();
    while (index < nr_items) {
        wxSizerItem *item = sizer->GetItem(index);
        wxPoint pos(item->GetPosition());
        wxSize size(item->GetSize());
        
        if (y >= last_y && y < pos.y + size.y/2)
            return (TrussThumbView *)item->GetWindow();
        last_y = pos.y + size.y/2;
                
        ++index;
    }
    
    return 0;
}

TrussThumbView *
DayView::insertBefore(TrussThumbView *view, Truss *truss) {
    TrussThumbView *nv = new TrussThumbView(this, truss);
        
    int index = 1, nr_items = sizer->GetItemCount();
    while (index < nr_items) {
        if (sizer->GetItem(index)->GetWindow() == view) {
            sizer->Insert(index, nv, 0, wxEXPAND | wxALL, 5);
            Layout();
            GetParent()->FitInside();
            return nv;
        }
        
        ++index;
    }
    
    nv->Destroy();
    return 0;
}

TrussThumbView *
DayView::insertAfter(TrussThumbView *view, Truss *truss) {
    TrussThumbView *nv = new TrussThumbView(this, truss);
    
    int index = 1, nr_items = sizer->GetItemCount();
    while (index < nr_items) {
        if (sizer->GetItem(index)->GetWindow() == view) {
            sizer->Insert(index+1, nv, 0, wxEXPAND | wxALL, 5);
            Layout();
            GetParent()->FitInside();
            return nv;
        }
        
        ++index;
    }
    
    nv->Destroy();
    return 0;
}

void DayView::onClick(wxMouseEvent &event) {
    ((WeekView *)GetParent())->deselect();
}

void DayView::onPaint(wxPaintEvent &event) {
    wxAutoBufferedPaintDC dc(this);
    
    const wxSize size = GetClientSize();
    wxPen oldPen(dc.GetPen());
    
    dc.SetBrush(dc.GetBackground());
    dc.SetPen(wxPen(dc.GetBackground().GetColour()));
    dc.DrawRectangle(size);
    
#ifndef __WXMSW__
    dc.SetPen(wxPen(wxColour(0,0,0)));
#else
    dc.SetPen(wxPen(
        wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVEBORDER)));
#endif
    //dc.DrawLine(0, size.y-1, size.x, size.y-1);
    if (leftBorder) {
        dc.DrawLine(0, 0, 0, size.y);
    }
}

// Used by the context (popup) menu
enum {
    kPopupImportHere = 1,
    kPopupPaste,
    kPopupPrintMe
};

void DayView::onRightClick(wxMouseEvent &event) {
    wxMenu menu;
    menu.Append(kPopupImportHere, wxT("Import .TRE file here..."));
    menu.AppendSeparator();
    menu.Append(kPopupPaste, wxT("Paste"));
    menu.AppendSeparator();
    menu.Append(kPopupPrintMe, wxT("Print this day..."));
    
    ((WeekView *)GetParent())->deselect();
    PopupMenu(&menu, event.GetPosition());
}

void DayView::onMenu(wxCommandEvent &event) {
    switch (int id = event.GetId()) {
    case kPopupImportHere:
        ((ScheduleFrame *)GetParent()->GetParent())->importTRE(daynr);
        break;
        
    case kPopupPaste:
        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject text;
                wxTheClipboard->GetData(text);
                Truss *truss = Truss::unpack(text.GetText().c_str());
                if (truss) {
                    const wxPoint &pos = ScreenToClient(wxGetMousePosition());
                    TrussThumbView *view = getViewAt(pos.x, pos.y);
                    TrussThumbView *newview;
                    if (!view)
                        newview = addThumbView(truss);
                    else
                        newview = insertBefore(view, truss);
                    
                    ((WeekView *)GetParent())->select(newview);
                }
            }
            
            wxTheClipboard->Close();
        }
        break;
        
    case kPopupPrintMe:
        printAll();
        break;
    }
}
