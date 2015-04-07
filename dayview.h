#ifndef dayview_h
#define dayview_h

#include <wx/panel.h>

#include <list>
#include <fstream>

class wxBoxSizer;
class TrussThumbView;
class Truss;
class TrussPrintout;

class DayView : public wxPanel {
    wxBoxSizer *sizer;
    bool leftBorder;
    int daynr;
        
public:
    DayView(wxWindow *parent, const wxString &text, bool leftBorder = false,
        int daynr = -1);
    ~DayView();
    
    TrussThumbView *addThumbView(Truss *truss);
    void removeThumbView(TrussThumbView *view);
    bool empty() const;
    
    void writeAll(std::ostream &out);
    void printAll();
    void addToPrintout(TrussPrintout &p) const;
    
    TrussThumbView *getViewAt(wxCoord x, wxCoord y);
    TrussThumbView *insertBefore(TrussThumbView *view, Truss *truss);
    TrussThumbView *insertAfter(TrussThumbView *view, Truss *truss);
    
    void onClick(wxMouseEvent &event);
    void onPaint(wxPaintEvent &event);
    void onRightClick(wxMouseEvent &event);
    void onMenu(wxCommandEvent &event);
};

#endif // dayview_h
