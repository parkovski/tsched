#ifndef bedview_h
#define bedview_h

#include <wx/panel.h>

class Truss;

class BedView : public wxPanel {
    Truss *truss;
    bool truss_owned;
    unsigned flags;
    
public:
    BedView(wxWindow *parent, Truss *truss);
    ~BedView();
    
    void setFlags(unsigned flags);

    void setTruss(Truss *truss, bool owned);
    Truss *getTruss(bool unown);
    
    void onPaint(wxPaintEvent &event);
    void onResize(wxSizeEvent &event);
};

#endif // bedview_h
