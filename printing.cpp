#include "printing.h"

#include <cassert>
#include <cstdio>

static int
get_text_height(wxDC &dc, const wxString &text) {
    int width, height;
    dc.GetTextExtent(text, &width, &height);
    
    // GetTextExtent only takes into account one line, so just count the \n's
    // and multiply the height by that.
    int text_len = text.length();
    int times = 1;
    for (int i = 0; i < text_len; ++i) {
        if (text[i] == wxT('\n'))
            ++times;
    }
    return times * height;
}

// On Windows, DrawText doesn't work with multiline text. This function also
// should take into account line wrapping.
static int
draw_ml(wxDC &dc, const wxString &text, int x, int y, int width) {
    int height = 0;
    int last_nl_pos = 0;
    int nl_pos = text.find('\n');
    wxString line;
    while (true) {
        line = text.substr(last_nl_pos, nl_pos);
        last_nl_pos = nl_pos + 1;
        
        if (line.empty()) {
            int lw, lh;
            dc.GetTextExtent(wxT("X"), &lw, &lh);
            height += lh;
        } else {
            dc.DrawText(line, x, y + height);
            
            int lw, lh;
            dc.GetTextExtent(line, &lw, &lh);
            height += lh;
        }
        
        if (nl_pos == -1)
            break;
        nl_pos = text.find('\n', last_nl_pos);
    }

    return height;
}

void
Page::print(wxDC &dc) {
    wxString wxs_header = header;
    int top;
    int header_width;
    int width = dc.GetSize().x - mleft - mright;
    dc.GetTextExtent(wxs_header, &header_width, &top);
    top += mtop;
    dc.DrawText(wxs_header, mleft + width / 2 - header_width / 2, mtop);
    
    for (int i = 0; i < trusses.size(); ++i) {
        printTruss(dc, trusses[i], top, width);
    }
}

extern wxBitmap *excl_icon;

void
Page::printTruss(wxDC &dc, Truss *truss, int &top, int width) {
    // Make sure the height is at least 8.5ft.
    float truss_height = truss->getHeight();
    int bed_height_px = 8.5f * 12.0f * width / (60.0f * 12.0f);
    if (truss_height < 8.5f * 12.0f)
        truss_height = 8.5f * 12.0f;
    int truss_height_px = truss_height * width / (60.0f * 12.0f);
    int bed_top_px = truss_height_px - bed_height_px;
    
    // Write the title above the box (or "Untitled" if none).
    const wxString &truss_text = truss->gettext();
    if (!truss_text.empty()) {
		top += draw_ml(dc, truss_text, mleft, top, width);
    } else {
		top += draw_ml(dc, wxT("Untitled"), mleft, top, width);
    }
    
    int empty_line_height = get_text_height(dc, wxT("X"));
    
    // If there are any notes, write them too (after a half-line break).
    const wxString &truss_notes = truss->getnotes();
    if (!truss_notes.empty()) {
        top += empty_line_height / 2;
        top += draw_ml(dc, truss_notes, mleft, top, width);
    }
    
    truss->draw(dc, mleft, top, width + mleft, top + truss_height_px);
    
    // Draw the truck bed, which is always 8.5ft.
    wxPoint lines[] = {
        wxPoint(mleft, top + bed_top_px),
            wxPoint(width + mleft, top + bed_top_px),
        wxPoint(mleft, top + bed_top_px), wxPoint(mleft, top + truss_height_px),
        wxPoint(mleft, top + truss_height_px),
            wxPoint(width + mleft, top + truss_height_px),
        wxPoint(width + mleft, top + bed_top_px),
            wxPoint(width + mleft, top + truss_height_px)
    };
    dc.DrawLines(8, lines);
        
    int tick_mark_height = empty_line_height * 0.8;

    float two_feet = width / 30.0f;
    float ten_feet = two_feet * 5;
    
    for (float x1 = mleft; x1 < width + mleft - 1; x1 += ten_feet) {
        dc.DrawLine((wxCoord)x1, top + bed_top_px,
            (wxCoord)x1, top + truss_height_px);
    
        float x2 = x1 + two_feet;
        for (unsigned nr = 0; nr < 4; ++nr, x2 += two_feet) {
            dc.DrawLine((wxCoord)x2, top + bed_top_px,
                (wxCoord)x2, top + bed_top_px + tick_mark_height);
            dc.DrawLine((wxCoord)x2, top + truss_height_px - tick_mark_height,
                (wxCoord)x2, top + truss_height_px);
        }
    }
    
	truss->drawGuides(dc, mleft, top, width + mleft, top + truss_height_px);
    
    // This section comes from bedview.cpp but is modified for the printout.
    int flags = truss->get();
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
            dc.GetTextExtent(wxT(">> FULL TRUCK <<"),
                &full_width, &full_height);
        if (flags & Truss::kOversize)
            dc.GetTextExtent(wxT(">> OVERSIZE LOAD <<"),
                &os_width, &os_height);
        if (flags & Truss::kVeryOversize)
            dc.GetTextExtent(
                wxT(">> SPECIAL PERMIT REQ'D <<"),
                &vos_width, &vos_height);
        
        overall_width = full_width;
        if (os_width > overall_width)
            overall_width = os_width;
        if (vos_width > overall_width)
            overall_width = vos_width;
        
        overall_height = full_height + os_height + vos_height;
        
        wxCoord left_pt = mleft + width - overall_width - 3;
        wxCoord top_pt = top + bed_top_px +
            (truss_height_px - bed_top_px - overall_height) / 2;
        
        // If there is enough room, center the messages in the non-used part.
        unsigned truss_right_ext = (truss->getWidth() + 6) * two_feet / 24.0f;
        if (truss_right_ext < left_pt)
            left_pt = truss_right_ext + (width - truss_right_ext) / 2
                - overall_width / 2;
        
        //dc.DrawBitmap(*excl_icon,
        //    left_pt + overall_width / 2 - excl_img_width / 2, top_pt);
        //top_pt += excl_img_height;
        if (flags & Truss::kFull) {
            dc.DrawText(wxT(">> FULL TRUCK <<"),
                left_pt + overall_width / 2 - full_width / 2, top_pt);
            top_pt += full_height;
        }
        if (flags & Truss::kOversize) {
            dc.DrawText(wxT(">> OVERSIZE LOAD <<"),
                left_pt + overall_width / 2 - os_width / 2, top_pt);
            top_pt += os_height;
        }
        if (flags & Truss::kVeryOversize) {
            dc.DrawText(wxT(">> SPECIAL PERMIT REQ'D <<"),
                left_pt + overall_width / 2 - vos_width / 2, top_pt);
        }
    }
    
    top += truss_height_px;
    
    // Use "triple spacing" as padding between trusses.
    top += empty_line_height * 3;
}

//-----

TrussPrintout::TrussPrintout(const char *day, Truss *truss)
                            : wxPrintout(wxT("Truss job schedule")) {
    
    // Create the first page.
    printitems.push_back(new PageBreak(day));
    
    if (truss)
        printitems.push_back(new TrussContainer(truss));
}

TrussPrintout::~TrussPrintout() {
    // In case printing was cancelled, we don't want to leak memory.
    for (int i = 0; i < printitems.size(); ++i) {
        if (printitems[i]->isbreak()) {
            delete static_cast<PageBreak *>(printitems[i]);
        } else {
            delete static_cast<TrussContainer *>(printitems[i]);
        }
    }
}

void
TrussPrintout::addTruss(Truss *truss) {
    printitems.push_back(new TrussContainer(truss));
}

void
TrussPrintout::pageBreak(const char *header) {
    printitems.push_back(new PageBreak(header));
}

// From app.cpp.
extern wxPageSetupDialogData *page_setup_dialog_data;

void
TrussPrintout::OnPreparePrinting() {
    int page_width, page_height;
    int current_page_y = 0;
    GetPageSizePixels(&page_width, &page_height);
    wxDC *dc = GetDC();
    
    int printer_x, printer_y, screen_x, screen_y;
    GetPPIPrinter(&printer_x, &printer_y);
    GetPPIScreen(&screen_x, &screen_y);
    float scale = printer_x / (float)screen_x;
    
    // Use a 12pt font. For some reason wxFont doesn't use real points, so
    // we still have to scale it.
    wxFont &font = *wxTheFontList->FindOrCreateFont(
        12 * scale, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    );
    dc->SetFont(font);
    
    // Margins:
    int ml, mt, mr, mb;
    int widthmm, heightmm;
    GetPageSizeMM(&widthmm, &heightmm);
    double one_mm_px = page_width / (double)widthmm;
    
    wxPoint margintl = page_setup_dialog_data->GetMarginTopLeft();
    wxPoint marginbr = page_setup_dialog_data->GetMarginBottomRight();
    
    ml = margintl.x * one_mm_px;
    mt = margintl.y * one_mm_px;
    mr = marginbr.x * one_mm_px;
    mb = marginbr.y * one_mm_px;
        
    page_height -= mt + mb;
    page_width -= ml + mr;
    
    for (int i = 0; i < printitems.size(); ++i) {
        if (printitems[i]->isbreak()) {
            PageBreak *pb = static_cast<PageBreak *>(printitems[i]);
            
            // Add room for the page header/title.
            current_page_y = get_text_height(*dc, pb->header);
            
            pages.push_back(Page(pb->header, ml, mt, mr, mb));
            
            delete pb;
        } else {
            Truss *truss = static_cast<TrussContainer *>(printitems[i])->truss;
            
            int truss_height_px = truss->getHeight() * page_width
                                  / (60.0 * 12.0);
            int total_height = truss_height_px
                               + get_text_height(*dc, truss->gettext());
            
            if (!truss->getnotes().empty()) {
                total_height += get_text_height(*dc, truss->getnotes());
                // Add the 1/2 line padding in between title and notes
                total_height += get_text_height(*dc, wxT("X")) / 2;
            }
                        
            if (current_page_y + total_height > page_height) {
                current_page_y = get_text_height(*dc, "continuation...");
                pages.push_back(Page("continuation...", ml, mt, mr, mb));
            }
            
            pages.back().add(truss);
            
            current_page_y += total_height;
            // Add 3 blank lines of padding before the next truss.
            current_page_y += get_text_height(*dc, wxT("X")) * 3;
            
            delete static_cast<TrussContainer *>(printitems[i]);
        }
    }
    
    printitems.clear();
}

void
TrussPrintout::GetPageInfo(int *minPage, int *maxPage,
                           int *pageFrom, int *pageTo) {

    *minPage = *pageFrom = 1;
    *maxPage = *pageTo = pages.size();
}

bool
TrussPrintout::HasPage(int pageNum) {
    // pageNum is 1..pages, whereas the vector is 0..pages-1.
    return pageNum >= 1 && pageNum <= pages.size();
}

bool
TrussPrintout::OnPrintPage(int pageNum) {
    assert(HasPage(pageNum)
        && "this should only be called if HasPage returns true");
    
    wxDC *dc = GetDC();
    if (!dc || !dc->IsOk())
        return false;
    
    int printer_x, printer_y, screen_x, screen_y;
    GetPPIPrinter(&printer_x, &printer_y);
    GetPPIScreen(&screen_x, &screen_y);
    float scale = printer_x / (float)screen_x;
    
    // wxFont is stupid apparently and uses "points" that don't actually match
    // up on the page.
    wxFont &f = *wxTheFontList->FindOrCreateFont(12 * scale,
        wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc->SetFont(f);
    
    pages[pageNum-1].print(*dc);
    
    return true;
}
