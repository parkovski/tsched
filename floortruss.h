#ifndef floortruss_h
#define floortruss_h

#include "truss.h"

#include <wx/string.h>

class FloorTruss : public Truss {
    float width;
    float height;
    
public:
    FloorTruss(float width);
    FloorTruss(float width, float height);
    
    virtual void draw(wxDC &dc, unsigned left, unsigned top, unsigned right,
        unsigned bottom);
    virtual void drawGuides(wxDC &dc, unsigned left, unsigned top,
        unsigned right, unsigned bottom);
    
    virtual float getWidth() const { return width; }
    virtual float getHeight() const { return height; }
    
    virtual wxString pack() const;
};

#endif // floortruss_h

