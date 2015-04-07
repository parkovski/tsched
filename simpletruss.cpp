#include "simpletruss.h"

#include <wx/dc.h>

#include <cstdio>

SimpleTruss::SimpleTruss(float base, float height)
    : Truss(height), base(base), height(height) { }

SimpleTruss::~SimpleTruss() { }

void SimpleTruss::draw(wxDC &dc, unsigned left, unsigned top, unsigned right,
        unsigned bottom) {
        
    // Trusses should be drawn 6" from the left side.
    float six_inches = (right-left) / (60.0f*2);
    
    float one_inch = (right-left) / 60.0f / 12.0f;
    
    float px_width = one_inch * base;
    float px_height = one_inch * height;
    
    wxPoint points[3] = {
        wxPoint(left + six_inches, bottom),
        wxPoint(left + six_inches + px_width/2, bottom - px_height),
        wxPoint(left + six_inches + px_width, bottom)
    };
    
    const wxBrush &old_brush = dc.GetBrush();
    dc.SetBrush(*wxGREY_BRUSH);
    dc.DrawPolygon(3, points);
    dc.SetBrush(old_brush);
}

void SimpleTruss::drawGuides(wxDC &dc, unsigned left, unsigned top,
    unsigned right, unsigned bottom) {
    
    float six_inches = (right-left) / (60.0f*2);
    
    float one_inch = (right-left) / 60.0f / 12.0f;
    
    float px_width = one_inch * base;
    float px_height = one_inch * height;
    
    wxCoord line_height, unused;
    dc.GetTextExtent(wxT("X"), &unused, &line_height);
    
    // height
    drawGuide(dc, left + six_inches + px_width/2 - line_height,
        bottom - px_height, px_height, height, true, line_height);
    
    // base
    drawGuide(dc, left + six_inches, bottom + line_height / 10, px_width,
        base, false, line_height);
    
    // tip to back of truck
    drawGuide(dc, left + six_inches + px_width / 2,
        bottom - px_height - line_height,
        right - left - six_inches - px_width / 2, 60*12 - 6 - base / 2, false,
        line_height);
    
    // base to back of truck
    drawGuide(dc, left + six_inches + px_width - 1, bottom + line_height / 10,
        right - left - six_inches - px_width, 60*12 - 6 - base, false,
        line_height);
}

wxString SimpleTruss::pack() const {
    wxString str(wxT("T"));
    str << base << wxT('x') << height;
    if (has(kFull))
        str << wxT('F');
    if (has(kOversize))
        str << wxT('O');
    if (has(kVeryOversize))
        str << wxT('P');
    
    str << escapeTextAndNotes();
    
    return str;
}
