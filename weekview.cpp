#include "weekview.h"
#include "dayview.h"
#include "simpletruss.h"
#include "floortruss.h"
#include "trussthumbview.h"
#include "frame.h"
#include "printing.h"

#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/msgdlg.h>

#include <fstream>
#include <string>
#include <cstdlib>

WeekView::WeekView(wxWindow *parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxFULL_REPAINT_ON_RESIZE), selection(0) {
    
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
    
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    
    monday = new DayView(this, wxT("Monday"), false, 0);
    sizer->Add(monday, 1, wxALL|wxEXPAND, 0);
    
    tuesday = new DayView(this, wxT("Tuesday"), true, 1);
    sizer->Add(tuesday, 1, wxALL|wxEXPAND, 0);
    
    wednesday = new DayView(this, wxT("Wednesday"), true, 2);
    sizer->Add(wednesday, 1, wxALL|wxEXPAND, 0);
    
    thursday = new DayView(this, wxT("Thursday"), true, 3);
    sizer->Add(thursday, 1, wxALL|wxEXPAND, 0);
    
    friday = new DayView(this, wxT("Friday"), true, 4);
    sizer->Add(friday, 1, wxALL|wxEXPAND, 0);
    
    saturday = new DayView(this, wxT("Saturday"), true, 5);
    sizer->Add(saturday, 1, wxALL|wxEXPAND, 0);
    
    SetScrollRate(10, 10);
    
    SetSizer(sizer);
}

void WeekView::putTruss(int day, Truss *truss) {
    DayView *dv;
    
    switch (day) {
    case 0:
        dv = monday;
        break;
    case 1:
        dv = tuesday;
        break;
    case 2:
        dv = wednesday;
        break;
    case 3:
        dv = thursday;
        break;
    case 4:
        dv = friday;
        break;
    case 5:
        dv = saturday;
        break;
    default:
        return;
    }
    
    dv->addThumbView(truss);
    ((ScheduleFrame *)GetParent())->setModifiedState();
}

void WeekView::select(TrussThumbView *newSelection) {
    if (selection)
        selection->select(false);
    
    selection = newSelection;
    if (selection) {
        selection->select(true);
        selection->SetFocus();
    } else
        this->SetFocus();
}

TrussThumbView *WeekView::getSelection() {
    return selection;
}

bool WeekView::empty() const {
    return monday->empty() && tuesday->empty() && wednesday->empty() &&
        thursday->empty() && friday->empty() && saturday->empty();
}

TrussPrintout *WeekView::printout() const {
    TrussPrintout *p = 0;
    
    DayView *views[] = {
        monday, tuesday, wednesday, thursday, friday, saturday
    };
    
    const char *titles[] = {
        "Jobs for Monday",
        "Jobs for Tuesday",
        "Jobs for Wednesday",
        "Jobs for Thursday",
        "Jobs for Friday",
        "Jobs for Saturday"
    };
    
    for (int i = 0; i < 6; ++i) {
        if (!views[i]->empty()) {
            if (!p)
                p = new TrussPrintout(titles[i]);
            else
                p->pageBreak(titles[i]);
            
            views[i]->addToPrintout(*p);
        }
    }
    
    return p;
}

bool WeekView::writeTo(const char *filename) {
    std::ofstream out(filename);
    if (!out.good())
        return false;
    
    out << "Schedule version 1\n";
    
    out << "+Monday\n";
    monday->writeAll(out);
    out << "+Tuesday\n";
    tuesday->writeAll(out);
    out << "+Wednesday\n";
    wednesday->writeAll(out);
    out << "+Thursday\n";
    thursday->writeAll(out);
    out << "+Friday\n";
    friday->writeAll(out);
    out << "+Saturday\n";
    saturday->writeAll(out);
    
    return true;
}

bool WeekView::readFrom(const char *filename) {
    std::ifstream in(filename);
    if (!in.good()) {
        wxMessageBox(wxT("Couldn't read from the schedule file."),
            wxT("Error"), wxOK);
        return false;
    }
    
    DayView *dv = 0;
    
    std::string line;
    std::getline(in, line);
    if (line.length() < 18 || line.substr(0, 17) != "Schedule version ") {
        wxMessageBox(wxT("Not a valid schedule file."), wxT("Error"), wxOK);
        return false;
    }
    
    double version;
    version = strtod(&line.c_str()[17], 0);
    if (version == 0) {
        wxMessageBox(wxT("Couldn't read schedule version."),
            wxT("Error"), wxOK);
        return false;
    }
        
    if (version > 1) {
        wxString msg(wxT("This program is not compatible ")
            wxT("with schedule version "));
        msg << version << wxT(". Maybe you need to upgrade?");
        wxMessageBox(
            msg,
            wxT("Error"),
            wxOK
        );
        return false;
    }
    
    do {
        std::getline(in, line);
        if (!line.length())
            continue;
        
        // To deal with line ending problems (CR/LF vs LF):
        if (line[line.length()-1] == '\r')
            line.erase(line.length()-1);
        
        if (line[0] == '+') {
            if (line == "+Monday")
                dv = monday;
            else if (line == "+Tuesday")
                dv = tuesday;
            else if (line == "+Wednesday")
                dv = wednesday;
            else if (line == "+Thursday")
                dv = thursday;
            else if (line == "+Friday")
                dv = friday;
            else if (line == "+Saturday")
                dv = saturday;
                        
            continue;
        }
        
        Truss *truss = Truss::unpack(line.c_str());
        if (truss && dv)
            dv->addThumbView(truss);
        else if (truss)
            delete truss;
        
    } while (in.good());
    
    return true;
}
