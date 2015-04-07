#include "simpletruss.h"
#include "floortruss.h"

#include <wx/string.h>
#include <wx/dc.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>

wxString Truss::packStripped() const {
    wxString packed = pack();
    int end_pos = -1;

    // By looking for the first semicolon, we will strip off the title and any
    // comments attached to the truss, suitable for storing it in the database.
    int semi_pos = packed.Find(';');
    if (semi_pos != -1)
        packed = packed.substr(0, semi_pos);
    
    // If there is an F/O/P, get rid of it too. Note that we really only want
    // to get rid of the F, since it is different for each job, but the O and P
    // stay the same so we will add them back on if they were there.
    int f_pos = packed.Find('F');
    int o_pos = packed.Find('O');
    int p_pos = packed.Find('P');
    
    if (f_pos != -1)
        end_pos = f_pos;
    if (o_pos != -1 && (end_pos == -1 || o_pos < end_pos))
        end_pos = o_pos;
    if (p_pos != -1 && (end_pos == -1 || p_pos < end_pos))
        end_pos = p_pos;

    if (end_pos != -1)
        packed = packed.substr(0, end_pos);

    if (o_pos != -1)
        packed += 'O';
    if (p_pos != -1)
        packed += 'P';

    return packed;
}

// A serialized truss is in the following format:
// 'T' or 'S' = triangle or square (simple/floor truss)
// <float>x<float> = width x height
// 'F' = full truck, 'O' = oversize, 'P' = special permit.
// ';' <job title/description text>.
Truss *Truss::unpack(const char *str) {
    Truss *truss;
    
    if (!str || str[0] == 0)
        return 0;

    float width, height = 0;
    char *end;
    if (str[0] == 'T' || str[0] == 'S') {
        errno = 0;
        width = strtod(&str[1], &end);
        if (end == &str[1] || errno)
            return 0;
        
        char *hstart;
        if (*end == 'x') {
            hstart = end+1;
            height = strtod(hstart, &end);
            if (hstart == end || errno)
                return 0;
        }
    } else {
        return 0;
    }
    
    if (str[0] == 'T')
        truss = new SimpleTruss(width, height);
    else if (str[0] == 'S')
        truss = height ? new FloorTruss(width, height) : new FloorTruss(width);
    else
        return 0;
    
    unsigned flags = 0;
    // end = the character after the height = start of flags.
    do {
        if (*end == 'F')
            flags |= Truss::kFull;
        else if (*end == 'O')
            flags |= Truss::kOversize;
        else if (*end == 'P')
            flags |= Truss::kVeryOversize;
        else
            break;
        
        ++end;
    } while (true);
    truss->set(flags);
    
    if (*end == ';') {
        ++end;
        truss->unescapeTextAndNotes(end);
    }
    
    return truss;
}

namespace {
    // measure specified in inches
    wxString nice_print_measurement(float measure) {
        wxString s;
        
        unsigned feet = measure / 12.0f;
        if (feet) {
            s << feet;
            s << '\'';
        }
        
        measure -= feet * 12;
        unsigned inches = measure;
        
        measure -= inches;
        unsigned frac = measure * 10000;
                
        /*
        // For 1/2, 1/4's, 1/8's, and 1/16's, write a fraction.
        const char * const sixteenths[] = {
            "", "1/16", "1/8", "3/16", "1/4", "5/16", "3/8", "7/16", "1/2",
            "9/16", "5/8", "11/16", "3/4", "13/16", "7/8", "15/16"
        };
        
        if (frac == 0) {
            if (inches)
                s << inches << wxT('"');
        } else if (frac % 625 == 0) {
            if (inches)
                s << inches << wxT(' ');
            s << sixteenths[frac/625] << wxT('"');
        } else {
            wxString extra;
            extra.Printf("%.3f\"", inches + measure);
            s << extra;
        }
        */
        // Round to the nearest inch:
        if (measure)
            ++inches;
        if (inches)
            s << inches << wxT('"');
        
        return s;
    }
}

void Truss::drawGuide(wxDC &dc, unsigned left, unsigned top, unsigned length,
    float inchLen, bool is_vertical, unsigned line_height) {
    
    wxString text(nice_print_measurement(inchLen));
    
    int textheight;
    int textwidth;
    dc.GetTextExtent(text, &textwidth, &textheight);
    
    dc.SetBrush(*wxWHITE_BRUSH);
    
    unsigned ends_len = line_height * 0.65;
    unsigned round = line_height * 0.2; // For the rounded rectangles.
    unsigned pad = 1 + line_height * 0.1;
    unsigned lesspad = 1 + line_height * 0.05;
        
    if (is_vertical) {
        bool text_horizontal = false;
        int seglen = length / 2 - textwidth / 2;
        if (seglen < 5) {
            text_horizontal = true;
            seglen = length / 2 - textheight / 2;
        }
        // Line segments:
        dc.DrawLine(left + textheight / 2, top,
            left + textheight / 2, top + seglen - 2);
        dc.DrawLine(left + textheight / 2, top + length - seglen + 2,
            left + textheight / 2, top + length);
        
        // Thingies on the ends:
        dc.DrawLine(left + textheight / 2 - ends_len / 2, top,
            left + textheight / 2 + ends_len / 2, top);
        dc.DrawLine(left + textheight / 2 - ends_len / 2, top + length,
            left + textheight / 2 + ends_len / 2, top + length);
        
        // If it's too cramped, draw the text horizontally instead.
        // Note the horizontal line uses textheight/2 as an adjustment anyways,
        // since the line drawing does that above.
        if (text_horizontal) {
            dc.DrawRoundedRectangle(left - textwidth / 2 + textheight / 2 - pad,
                top + length / 2 - textheight / 2 - pad, textwidth + 2 * pad,
                textheight + pad, round);
            dc.DrawText(text, left - textwidth / 2 + textheight / 2,
                top + length / 2 - textheight / 2);
        } else {
            dc.DrawRoundedRectangle(left,
                top + length / 2 - textwidth / 2 - pad,
                textheight + pad, textwidth + 2 * pad, round);
            dc.DrawRotatedText(text, left,
                top + length / 2 + textwidth / 2 + 2, 90);
            // Add 2 because at small resolutions, sometimes the ' gets cut
            // off, though it looks fine at high resolutions (printed) where
            // the 2 won't really have any effect.
        }
    } else {
        int seglen = length / 2 - textwidth / 2;
        dc.DrawLine(left, top + textheight / 2,
            left + seglen - pad, top + textheight / 2);
        dc.DrawLine(left + length - seglen + pad, top + textheight / 2,
            left + length, top + textheight / 2);
        
        dc.DrawLine(left, top + textheight / 2 - ends_len / 2,
            left, top + textheight / 2 + ends_len / 2);
        dc.DrawLine(left + length, top + textheight / 2 - ends_len / 2,
            left + length, top + textheight / 2 + ends_len / 2);
        
        dc.DrawRoundedRectangle(left + length / 2 - textwidth / 2 - pad,
            top - pad, textwidth + 2 * pad, textheight + pad, round);
        dc.DrawText(text, left + length / 2 - textwidth / 2, top);
    }
}

wxString Truss::escapeTextAndNotes() const {
    if (text.empty() && notes.empty())
        return wxT("");

    wxString ret(wxT(";"));
    
    unsigned len = text.length();
    for (unsigned i = 0; i < len; ++i) {
#if defined(__GNUC__)
        switch ((typeof(wxT(' ')))text[i]) {
#elif defined(_MSC_VER)
		switch ((decltype(wxT(' ')))text[i]) {
#else
#error Sorry, your compiler needs some sort of typeof feature!
#endif
        case wxT(';'):
            ret += wxT("\\;");
            break;
            
        case wxT('\r'):
            continue;
            
        case wxT('\n'):
            ret += wxT("\\n");
            break;
        
        case wxT('\\'):
            ret += wxT("\\\\");
            break;
        
        default:
            ret += text[i];
        }
    }
    
    len = notes.length();
    if (!len)
        return ret;
    
    ret += wxT(';');
    
    for (unsigned i = 0; i < len; ++i) {
#if defined(__GNUC__)
        switch ((typeof(wxT(' ')))notes[i]) {
#elif defined(_MSC_VER)
		switch ((decltype(wxT(' ')))notes[i]) {
#else
#error See above - you need a typeof intrinsic.
#endif
        case wxT(';'):
            ret += wxT("\\;");
            break;
            
        case wxT('\r'):
            continue;
            
        case wxT('\n'):
            ret += wxT("\\n");
            break;
        
        case wxT('\\'):
            ret += wxT("\\\\");
            break;
        
        default:
            ret += notes[i];
        }
    }
    
    return ret;
}

void Truss::unescapeTextAndNotes(const char *serialized) {
    text.clear();
    notes.clear();
    
    wxString *appending = &text;
    unsigned i = 0;
    while (true) {
        switch (serialized[i]) {
        case '\\':
            ++i;
            switch (serialized[i]) {
            case '\\':
                *appending += wxT('\\');
                break;
            
            case ';':
                *appending += wxT(';');
                break;
                
            case 'n':
                *appending += wxT('\n');
                break;
                
            default:
                break;
            }
            
            break;
            
        case ';':
            if (appending == &text)
                appending = &notes;
            break;
        
        case 0:
            return;
        
        default:
            *appending += serialized[i];
            break;
        }
        
        ++i;
    }
    
    // Return inside the while loop, when null character found.
}
