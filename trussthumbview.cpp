#include "trussthumbview.h"
#include "bedviewdlg.h"
#include "truss.h"
#include "weekview.h"
#include "dayview.h"
#include "frame.h"
#include "printing.h"
#include "dbdialog.h"

#include <wx/dcbuffer.h>
#include <wx/dataobj.h>
#include <wx/dnd.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/pen.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/bitmap.h>

#include <cstdlib>

TrussThumbView::TrussThumbView(wxWindow *parent, Truss *truss)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(130, 75),
            wxWANTS_CHARS),
        truss(truss), mx(-1), is_selected(false) {
    
    if (!truss->getnotes().empty())
        SetToolTip(truss->getnotes());
        
    SetBackgroundColour(*wxWHITE);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    Bind(wxEVT_PAINT, &TrussThumbView::paintEvt, this);
    Bind(wxEVT_LEFT_DCLICK, &TrussThumbView::dblClick, this);
    Bind(wxEVT_LEFT_DOWN, &TrussThumbView::mouseDown, this);
    Bind(wxEVT_MOTION, &TrussThumbView::mouseMove, this);
    Bind(wxEVT_LEFT_UP, &TrussThumbView::mouseUp, this);
    Bind(wxEVT_KEY_DOWN, &TrussThumbView::keyDown, this);
    Bind(wxEVT_RIGHT_DOWN, &TrussThumbView::rightClick, this);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &TrussThumbView::menuClick, this);
}

TrussThumbView::~TrussThumbView() {
    delete truss;
}

bool TrussThumbView::Destroy() {
    ((DayView *)GetParent())->removeThumbView(this);
    ((ScheduleFrame *)GetParent()->GetParent()->GetParent())
        ->setModifiedState();
    return wxWindow::Destroy();
}

void TrussThumbView::checkedDestroy() {
    if (wxYES == wxMessageBox(
        wxT("Do you really want to delete this truss: \"") +
        truss->gettext() +
        wxT("\"?\n")
        wxT("(To suppress this warning, hold shift while pressing delete)."),
        wxT("Warning: Operation cannot be undone."),
        wxYES_NO | wxICON_EXCLAMATION)) {
        
        wxWindow *parent = GetParent();
        ((WeekView *)parent->GetParent())->deselect();
        Destroy();
        parent->Layout();
        parent->GetParent()->FitInside();
    }
}

void TrussThumbView::select(bool selected) {
    if (selected == is_selected)
        return;
    
    is_selected = selected;
    if (selected)
        SetBackgroundColour(
            wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    else
        SetBackgroundColour(*wxWHITE);
    
    Refresh();
}

void TrussThumbView::copyToClipboard() {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(truss->pack()));
        wxTheClipboard->Close();
    }
}

// Defined/initialized in app.cpp
extern wxBitmap *excl_icon_small, *full_icon, *oversize_icon, *notes_icon;

void TrussThumbView::paintEvt(wxPaintEvent &event) {
    wxAutoBufferedPaintDC dc(this);
    
    //dc.Clear();
    wxSize size(GetClientSize());
    
    // Draw a border & clear the window
    dc.SetBrush(dc.GetBackground());
    dc.DrawRectangle(size);
    
    if (is_selected) {
        wxColour clr(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
        //dc.SetPen(wxPen(clr));
        dc.SetTextForeground(clr);
    }
    
    wxCoord height; // text height
        
    // Use fitString to write "..." if the string is too long.
    const wxString titleText(truss->gettext());
    if (titleText.empty())
        dc.DrawText(fitString(dc, wxT("Untitled"), size.x - 6, &height), 3, 3);
    else
        dc.DrawText(fitString(dc, titleText, size.x - 6, &height), 3, 3);
    
    // Draw the underline separating the label from the truss drawing.
    dc.DrawLine(1, height + 3, size.x - 1, height + 3);
    
    // Draw the truck bed (5px padding)
    float one_foot = (size.x - 10) / 60.0f;
    unsigned bed_height = one_foot * 8.5f;
    
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.DrawRectangle(5, size.y - 5 - bed_height, size.x - 10, bed_height+1);
    truss->draw(dc, 5, size.y - 5 - bed_height, size.x - 5, size.y - 5);
    
    // If there are any flags set, show the appropriate icons.
    unsigned icons_width = 0, full_width = 0, ovs_width = 0, excl_width = 0,
        notes_width = 0;
    bool has_notes = !truss->getnotes().empty();
    
    if (truss->get() || has_notes) {
        if (truss->has(Truss::kFull))
            icons_width += full_width = full_icon->GetWidth();
        if (truss->has(Truss::kOversize))
            icons_width += ovs_width = oversize_icon->GetWidth();
        if (truss->has(Truss::kVeryOversize))
            icons_width += excl_width = excl_icon_small->GetWidth();
        if (has_notes)
            icons_width += notes_width = notes_icon->GetWidth();
        
        unsigned x = size.x / 2 - icons_width / 2;
        if (has_notes) {
            dc.DrawBitmap(*notes_icon, x, height + 3 + 3);
            x += notes_width;
        }
        if (full_width) {
            dc.DrawBitmap(*full_icon, x, height + 3 + 3);
            x += full_width;
        }
        if (ovs_width) {
            dc.DrawBitmap(*oversize_icon, x, height + 3 + 3);
            x += ovs_width;
        }
        if (excl_width) {
            dc.DrawBitmap(*excl_icon_small, x, height + 3 + 3);
        }
    }
}

void TrussThumbView::dblClick(wxMouseEvent &event) {
    // A null parent dialog will be assigned to the main window.
    BedViewDlg dialog(0, truss);
    
    if (dialog.ShowModal() == wxID_OK) {
        if (truss->gettext() == dialog.getTitleText()
            && truss->getnotes() == dialog.getNotesText()
            && truss->get() == dialog.getTrussFlags()) {
            
            // Nothing changed.
            return;
        }
    
        truss->settext(dialog.getTitleText());
        truss->setnotes(dialog.getNotesText());
        if (dialog.getNotesText().empty())
            UnsetToolTip();
        else
            SetToolTip(dialog.getNotesText());
        truss->set(dialog.getTrussFlags());
        ((ScheduleFrame *)GetParent()->GetParent()->GetParent())
            ->setModifiedState();
        Refresh();
    }
}

void TrussThumbView::mouseDown(wxMouseEvent &event) {
    mx = event.GetX();
    my = event.GetY();
    
    WeekView *wv = (WeekView *)GetParent()->GetParent();
    wv->select(this);
}

void TrussThumbView::mouseMove(wxMouseEvent &event) {
    if (!event.Dragging() || mx == -1)
        return;
    
    wxCoord dx, dy;
    dx = abs(event.GetX() - mx);
    dy = abs(event.GetY() - my);
    
    if (dx >= wxSystemSettings::GetMetric(wxSYS_DRAG_X) ||
        dy >= wxSystemSettings::GetMetric(wxSYS_DRAG_Y)) {
        
        mx = -1;
                
        wxTextDataObject text(truss->pack());
        wxDropSource source(text, this);
        wxDragResult result = source.DoDragDrop(wxDrag_DefaultMove);
        
        wxWindow *parent = GetParent();
        
        // For some reason wx cocoa doesn't do the default move thing.
#ifndef __WXOSX_COCOA__
        if (result == wxDragMove)
#endif
            Destroy();
        
        parent->Layout();
        parent->GetParent()->FitInside();
    }
}

void TrussThumbView::mouseUp(wxMouseEvent &event) {
    mx = -1;
}

void TrussThumbView::keyDown(wxKeyEvent &event) {
    int code = event.GetKeyCode();
    if (code == WXK_BACK || code == WXK_DELETE) {
        if (event.ShiftDown()) {
            wxWindow *parent = GetParent();
            ((WeekView *)parent->GetParent())->deselect();
            Destroy();
            parent->Layout();
            parent->GetParent()->FitInside();
        } else {
            checkedDestroy();
        }
            
        return;
    }
    event.Skip();
}

enum {
    kPopupEdit = 1,

    kPopupImportToDatabase,
    
    kPopupCut,
    kPopupCopy,
    kPopupPaste,
    kPopupDelete,
    
    kPopupPrint
};

void TrussThumbView::rightClick(wxMouseEvent &event) {
    wxMenu menu;
    menu.Append(kPopupEdit, wxT("Edit..."));
    menu.AppendSeparator();
    menu.Append(kPopupImportToDatabase, wxT("Import to database..."));
    menu.AppendSeparator();
    menu.Append(kPopupCut, wxT("Cut"));
    menu.Append(kPopupCopy, wxT("Copy"));
    menu.Append(kPopupPaste, wxT("Paste"));
    menu.Append(kPopupDelete, wxT("Delete"));
    menu.AppendSeparator();
    menu.Append(kPopupPrint, wxT("Print this truss..."));
    
    ((WeekView *)GetParent()->GetParent())->select(this);
    PopupMenu(&menu, event.GetPosition());
}

// Defined in app.cpp
extern wxPrintData *print_data;

void TrussThumbView::menuClick(wxCommandEvent &event) {
    int id = event.GetId();
    switch (id) {
    case kPopupEdit: {
        // Copied from double click.
        BedViewDlg dialog(0, truss);
        
        if (dialog.ShowModal() == wxID_OK) {
            if (truss->gettext() == dialog.getTitleText()
                && truss->getnotes() == dialog.getNotesText()
                && truss->get() == dialog.getTrussFlags()) {
                
                // Nothing changed.
                return;
            }
        
            truss->settext(dialog.getTitleText());
            truss->setnotes(dialog.getNotesText());
            if (dialog.getNotesText().empty())
                UnsetToolTip();
            else
                SetToolTip(dialog.getNotesText());
            truss->set(dialog.getTrussFlags());
            ((ScheduleFrame *)GetParent()->GetParent()->GetParent())
                ->setModifiedState();
            Refresh();
        }
    }
        break;
        
    case kPopupImportToDatabase: 
        db_try_import(GetParent()->GetParent()->GetParent(), truss);
        break;

    case kPopupCut: {
        copyToClipboard();
        // Copied from delete key pressed.
        wxWindow *parent = GetParent();
        ((WeekView *)parent->GetParent())->deselect();
        Destroy();
        parent->Layout();
        parent->GetParent()->FitInside();
    }
        break;
        
    case kPopupCopy:
        copyToClipboard();
        break;
        
    case kPopupPaste:
        // Copied from ScheduleFrame paste.
        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject text;
                wxTheClipboard->GetData(text);
                Truss *truss = Truss::unpack(text.GetText().c_str());
                if (truss) {
                    ((DayView *)GetParent())->insertAfter(this, truss);
                    ((ScheduleFrame *)GetParent()->GetParent()->GetParent())
                        ->setModifiedState();
                }
            }
            
            wxTheClipboard->Close();
        }
        break;
        
    case kPopupDelete: {
        // Copied from delete key pressed.
        wxWindow *parent = GetParent();
        ((WeekView *)parent->GetParent())->deselect();
        Destroy();
        parent->Layout();
        parent->GetParent()->FitInside();
    }
        break;
    
    case kPopupPrint: {
        TrussPrintout p("Single Truss Printout", truss);
        wxPrintDialogData data(*print_data);
        wxPrinter printer(&data);
        bool success = printer.Print(this, &p, true);
    }
        break;
    }
}

// Try to fit a string into a given width. If it doesn't fit, it will be
// replaced with whatever part does fit, followed by "..."
// It also measures the height for convenience to draw a line below it.
wxString TrussThumbView::fitString(wxDC &dc, const wxString &str,
        wxCoord dstWidth, wxCoord *outHeight) {
    
    wxCoord width, height, descent, externalLeading;
    dc.GetTextExtent(str, &width, &height, &descent, &externalLeading);
    if (outHeight)
        *outHeight = height + descent + externalLeading;
    if (width <= dstWidth)
        return str;
        
    // Now the fun starts. First get an approximate number of characters
    // based on the average, then keep adding or subtracting characters until
    // we find the right amount.
    
    // Make sure there is room for the '...'
    wxCoord ellipsisWidth;
    dc.GetTextExtent(wxT("..."), &ellipsisWidth, &height);
    dstWidth -= ellipsisWidth;
    
    // There is no possibility of division by zero here, since a zero length
    // string would always return at (width <= dstWidth).
    unsigned approx = dstWidth / ((unsigned)width / str.length());
    
    // If old width is less than dstWidth, add a character to find new width.
    // If it is greater, subtract a character.
    // Then, if old and new are both greater or less, loop again.
    // Otherwise, return the shorter of the two.
    wxString oldStr;
    wxString newStr(str.Left(approx));
    wxCoord oldStrWidth, newStrWidth;
    dc.GetTextExtent(newStr, &newStrWidth, &height);
    unsigned index = approx;
    while (true) {
        oldStr = newStr;
        oldStrWidth = newStrWidth;
        
        if (oldStrWidth == dstWidth) {
            return oldStr + wxT("...");
        } else if (oldStrWidth < dstWidth) {
            newStr += str[index++];
            dc.GetTextExtent(newStr, &newStrWidth, &height);
            if (newStrWidth <= dstWidth)
                continue;
                        
            return oldStr + wxT("...");
        } else {
            // oldStr is too big.
            
            newStr.erase(newStr.length()-1);
            dc.GetTextExtent(newStr, &newStrWidth, &height);
            if (newStrWidth > dstWidth)
                continue;
            
            return newStr + wxT("...");
        }
    }
}
