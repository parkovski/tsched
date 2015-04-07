#include "floortruss.h"

#include <wx/dc.h>

FloorTruss::FloorTruss(float width)
    : Truss(13*3.5f), width(width), height(13*3.5f) {
}

FloorTruss::FloorTruss(float width, float height)
    : Truss(height), width(width), height(height) {
}

void FloorTruss::draw(wxDC &dc, unsigned left, unsigned top, unsigned right,
        unsigned bottom) {
        
    // Trusses should be drawn 6" from the left side.
    float six_inches = (right-left) / (60.0f*2);
    
    float one_inch = (right-left) / 60.0f / 12.0f;
    
    unsigned px_width = one_inch * width;
    unsigned px_height = one_inch * height;
    
    const wxBrush &old_brush = dc.GetBrush();
    dc.SetBrush(*wxGREY_BRUSH);
    dc.DrawRectangle(left + six_inches, bottom - px_height, px_width+1,
        px_height+1);
    dc.SetBrush(old_brush);
}

void FloorTruss::drawGuides(wxDC &dc, unsigned left, unsigned top,
    unsigned right, unsigned bottom) {
    
    float six_inches = (right-left) / (60.0f*2);
    
    float one_inch = (right-left) / 60.0f / 12.0f;
    
    unsigned px_width = one_inch * width;
    unsigned px_height = one_inch * height;
    
    wxCoord line_height, unused;
    dc.GetTextExtent(wxT("X"), &unused, &line_height);
    
    // height
    drawGuide(dc, left + six_inches + px_width/2 - line_height / 2,
        bottom - px_height, px_height, height, true, line_height);
    
    // base
    drawGuide(dc, left + six_inches, bottom + line_height / 10, px_width,
        width, false, line_height);
    
    // base to back of truck
    drawGuide(dc, left + six_inches + px_width, bottom + line_height / 10,
        right - left - six_inches - px_width, 60*12 - 6 - width, false,
        line_height);
}

wxString FloorTruss::pack() const {
    wxString str(wxT("S"));
    str << width << wxT('x') << height;
    if (has(kFull))
        str << wxT('F');
    if (has(kOversize))
        str << wxT('O');
    if (has(kVeryOversize))
        str << wxT('P');
    
    str << escapeTextAndNotes();
    
    return str;
}

