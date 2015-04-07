#ifndef simpletruss_h
#define simpletruss_h

#include "truss.h"

#include <wx/string.h>

class SimpleTruss : public Truss {
    // these are inch measurements.
    float base;
    float height;
    
public:
    SimpleTruss(float base, float height);
    virtual ~SimpleTruss();
    
    virtual void draw(wxDC &dc, unsigned left, unsigned top, unsigned right,
        unsigned bottom);
    virtual void drawGuides(wxDC &dc, unsigned left, unsigned top,
        unsigned right, unsigned bottom);
        
    virtual float getWidth() const { return base; }
    virtual float getHeight() const { return height; }
    
    virtual wxString pack() const;
};

#endif // simpletruss_h

