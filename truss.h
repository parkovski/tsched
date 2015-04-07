#ifndef truss_h
#define truss_h

#include <wx/string.h>

class wxDC;

class Truss {
protected:
    unsigned flags;
    wxString text;
    wxString notes;
    
public:
    enum {
        kFull = 1,
        kOversize = 2,
        kVeryOversize = 4
    };

    Truss(float height)
        : flags((height >= 12*8.5f) ?
            (height >= 12*14.0f) ? kOversize | kVeryOversize : kOversize
            : 0), text(wxT("")), notes(wxT("")) { }
    virtual ~Truss() { }
    
    void set(unsigned flag) { flags = flag; }
    bool has(unsigned flag) const { return flags & flag; }
    unsigned get() const { return flags; }
    
    void settext(const wxString &text) { this->text = text; }
    const wxString gettext() const { return text; }
    
    void setnotes(const wxString &notes) { this->notes = notes; }
    const wxString getnotes() const { return notes; }
    
    virtual float getWidth() const = 0;
    virtual float getHeight() const = 0;

    // draws a truss to a wxDC, inside a 60' x 8.5' rectangle.
    virtual void draw(wxDC &dc, unsigned left, unsigned top, unsigned right,
        unsigned bottom) = 0;
    virtual void drawGuides(wxDC &dc, unsigned left, unsigned top,
        unsigned right, unsigned bottom) = 0;
    
    // draws a measurement guide.
    void drawGuide(wxDC &dc, unsigned left, unsigned top, unsigned length,
        float inchLen, bool is_vertical, unsigned line_height);
        
    wxString escapeTextAndNotes() const;
    void unescapeTextAndNotes(const char *serialized);
    
    virtual wxString pack() const = 0;
    wxString packStripped() const;
    static Truss *unpack(const char *str);
};

#endif // truss_h
