#include "bedview.h"
#include "truss.h"

#include <wx/dcbuffer.h>
#include <wx/bitmap.h>

#include <stdio.h>

namespace {
    // Don't use the extra space if we don't need it. The professional thing
    // would be to scale the minimum size based on window size and truss height.
    wxSize get_default_size(Truss *truss) {
        if (!truss)
            return wxSize(840, 168);

        if (truss->getHeight() <= 8.5f*12)
            return wxSize(840, 168);
        else
            return wxSize(840, 224);
    }
}

// A trailer is 60' by 8.5'. However, a truss height can exceed 14' in some
// cases.
BedView::BedView(wxWindow *parent, Truss *truss)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, get_default_size(truss),
        wxBORDER_SIMPLE), truss(truss), flags(truss ? truss->get() : 0),
        truss_owned(false) {
    
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &BedView::onPaint, this);
    Bind(wxEVT_SIZE, &BedView::onResize, this);
}

BedView::~BedView() {
    if (truss_owned)
        delete truss;
}

void BedView::setFlags(unsigned flags) {
    this->flags = flags;
    Refresh();
}

void BedView::setTruss(Truss *truss, bool owned) {
    if (truss_owned)
        delete this->truss;

    this->truss = truss;

    if (truss)
        this->truss_owned = owned;

    Refresh();
}

Truss *BedView::getTruss(bool unown) {
    if (unown)
        truss_owned = false;
    return truss;
}

extern wxBitmap *excl_icon;

void BedView::onPaint(wxPaintEvent &event) {
    wxAutoBufferedPaintDC dc(this);
    wxSize size(GetClientSize());

    dc.Clear();
    
    // The width will always be 60'. The height can change based on panel size.
    float height_in_ft = size.y / (size.x / 60.0f);
    
    // The truck bed is 8.5', and should be drawn 1' from the bottom of the
    // panel (but a minimum of 20px).
    float one_foot = size.y / height_in_ft;
    unsigned bed_base = one_foot >= 20 ? size.y - one_foot : size.y - 20;
    unsigned bed_top = bed_base - one_foot * 8.5;
    
    // Draw lines every 10' and tick marks every 2'.
    float ten_feet = size.x / 6.0f;
    float two_feet = size.x / 30.0f;
    
    dc.DrawRectangle(0, bed_top, size.x, bed_base-bed_top+1);
    
    // If there is no truss, we still want to draw the guides.
    if (truss)
        truss->draw(dc, 0, bed_top, size.x, bed_base);
    
    // Redraw the lines around the rectangle, since the truss may have drawn
    // over them. Drawing the left or bottom lines is unnecessary, however.
    wxPoint lines[] = {
        wxPoint(0, bed_top), wxPoint(size.x, bed_top),
        wxPoint(size.x, bed_top), wxPoint(size.x, bed_base),
    };
    dc.DrawLines(4, lines);
    
    for (float x1 = 0; x1 < size.x; x1 += ten_feet) {
        dc.DrawLine((wxCoord)x1, bed_top, (wxCoord)x1, bed_base);
    
        float x2 = x1 + two_feet;
        for (unsigned nr = 0; nr < 4; ++nr, x2 += two_feet) {
            dc.DrawLine((wxCoord)x2, bed_top, (wxCoord)x2, bed_top + 10);
            dc.DrawLine((wxCoord)x2, bed_base - 10, (wxCoord)x2, bed_base);
        }
    }

    if (!truss)
        return;
    
    truss->drawGuides(dc, 0, bed_top, size.x, bed_base);
    
    if (flags) {
        // Find the widths and heights of each message to be displayed.
        wxCoord full_width = 0, full_height = 0;
        wxCoord os_width = 0, os_height = 0;
        wxCoord vos_width = 0, vos_height = 0;
        wxCoord overall_width, overall_height;
        int excl_img_width, excl_img_height;
        
        excl_img_width = excl_icon->GetWidth();
        excl_img_height = excl_icon->GetHeight();
        
        if (flags & Truss::kFull)
            dc.GetTextExtent(wxT(">> F U L L   T R U C K <<"),
                &full_width, &full_height);
        if (flags & Truss::kOversize)
            dc.GetTextExtent(wxT(">> O V E R S I Z E   L O A D <<"),
                &os_width, &os_height);
        if (flags & Truss::kVeryOversize)
            dc.GetTextExtent(
                wxT(">> OVERSIZE LOAD - SPECIAL PERMIT REQUIRED <<"),
                &vos_width, &vos_height);
        
        overall_width = full_width;
        if (os_width > overall_width)
            overall_width = os_width;
        if (vos_width > overall_width)
            overall_width = vos_width;
        
        overall_height = full_height + os_height + vos_height;
        
        wxCoord left_pt = size.x - overall_width - 3;
        wxCoord top_pt = bed_top +
            (bed_base - bed_top - excl_img_height - overall_height) / 2;
        
        // If there is enough room, center the messages in the non-used part.
        unsigned truss_right_ext = (truss->getWidth() + 6) * one_foot / 12.0f;
        if (truss_right_ext < left_pt)
            left_pt = truss_right_ext + (size.x - truss_right_ext) / 2
                - overall_width / 2;
        
        dc.DrawBitmap(*excl_icon,
            left_pt + overall_width / 2 - excl_img_width / 2, top_pt);
        top_pt += excl_img_height;
        if (flags & Truss::kFull) {
            dc.DrawText(wxT(">> F U L L   T R U C K <<"),
                left_pt + overall_width / 2 - full_width / 2, top_pt);
            top_pt += full_height;
        }
        if (flags & Truss::kOversize) {
            dc.DrawText(wxT(">> O V E R S I Z E   L O A D <<"),
                left_pt + overall_width / 2 - os_width / 2, top_pt);
            top_pt += os_height;
        }
        if (flags & Truss::kVeryOversize) {
            dc.DrawText(wxT(">> OVERSIZE LOAD - SPECIAL PERMIT REQUIRED <<"),
                left_pt + overall_width / 2 - vos_width / 2, top_pt);
        }
    }
}

void BedView::onResize(wxSizeEvent &event) {
    Refresh();
}
