#ifndef weekview_h
#define weekview_h

#include <wx/scrolwin.h>

class DayView;
class Truss;
class TrussThumbView;
class TrussPrintout;

class WeekView : public wxScrolledWindow {
    DayView *monday;
    DayView *tuesday;
    DayView *wednesday;
    DayView *thursday;
    DayView *friday;
    DayView *saturday;
    TrussThumbView *selection;
    
public:
    WeekView(wxWindow *parent);
    
    void putTruss(int day, Truss *truss);
    
    void select(TrussThumbView *newSelection);
    TrussThumbView *getSelection();
    void deselect() { select(0); }
        
    bool empty() const;
    
    // Must delete it when you are done.
    TrussPrintout *printout() const;
    
    bool writeTo(const char *filename);
    bool readFrom(const char *filename);
};

#endif // weekview_h
