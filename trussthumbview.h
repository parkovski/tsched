#ifndef trussthumbview_h
#define trussthumbview_h

#include <wx/panel.h>

class Truss;
class wxDC;

class TrussThumbView : public wxWindow {
    Truss *truss; // TrussThumbView owns the Truss.
    wxCoord mx, my; // Mouse coordinates for drag/drop.
    bool is_selected;
    
public:
    TrussThumbView(wxWindow *parent, Truss *truss);
    ~TrussThumbView();
    
    virtual bool Destroy();
    
    // Asks if the user wants to destroy this object.
    void checkedDestroy();
    
    void select(bool selected);
    
    void copyToClipboard();
    Truss *getTruss() { return truss; }
    
    void paintEvt(wxPaintEvent &event);
    void dblClick(wxMouseEvent &event);
    void mouseDown(wxMouseEvent &event);
    void mouseMove(wxMouseEvent &event);
    void mouseUp(wxMouseEvent &event);
    void keyDown(wxKeyEvent &event);
    void rightClick(wxMouseEvent &event);
    void menuClick(wxCommandEvent &event);
    
    static wxString fitString(wxDC &dc, const wxString &str,
        wxCoord width, wxCoord *outHeight);
};

#endif // trussthumbview_h
