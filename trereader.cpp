#include "trereader.h"
#include "simpletruss.h"
#include "floortruss.h"

#include <string>
#include <cstdlib>
#include <cerrno>
#include <cmath>

TREReader::TREReader(const char *filename)
    : in(filename), error(e_uninitialized) {
}

TREReader::~TREReader() {
    in.close();
}

enum tre_section {
    sec_none,
    sec_lumber_info,
    sec_truss_info
};

Truss *TREReader::getTruss() {
    if (!in.good()) {
        error = e_cantopen;
        return 0;
    }
    
    std::string line;
    tre_section sec = sec_none;
    truss_type tt = tt_invalid;
    
    ts = 0;
    span = 0;
    truss_height = 0;
    float left_overhang = 0, right_overhang = 0;
    
    do {
        std::getline(in, line);
        if (line[0] == '[') {
            // Change sections...
            if (line == "[ADDITIONAL LUMBER INFO]")
                sec = sec_lumber_info;
            else if (line == "[ADDITIONAL TRUSS INFO]")
                sec = sec_truss_info;
            else
                sec = sec_none;
            
            continue;
        }
        
        if (sec == sec_none)
            continue;
        
        size_t equal_pos;
        if ((equal_pos = line.find('=')) != std::string::npos) {
            std::string::const_iterator it = line.begin();
            std::string key(it, it + equal_pos);
            
            switch (sec) {
            case sec_lumber_info:
                if (key == "TS") {
                    errno = 0;
                    const char *start, *end;
                    start = &line.c_str()[equal_pos+1];
                    ts = strtod(start, const_cast<char **>(&end));
                    if (start == end || errno) {
                        error = e_illegaltruss;
                        return 0;
                    }
                }
                break;
            
            case sec_truss_info:
                if (key == "TRUSS TYPE") {
                    if (line.length() > 11)
                        type = std::string(line.substr(11));
                    else
                        type = "";
                
                    if (type == "COMMON")
                        tt = tt_common;
                    else if (type == "Common Truss")
                        tt = tt_common;
                    else if (type == "MONOPITCH")
                        tt = tt_monopitch;
                    else if (type == "Monopitch Truss")
                        tt = tt_monopitch;
                    else if (type == "FLOOR")
                        tt = tt_floor;
                    else if (type == "Floor Truss")
                        tt = tt_floor;
                } else if (key == "Span") {
                    errno = 0;
                    const char *start, *end;
                    start = &line.c_str()[equal_pos+1];
                    span = strtod(start, const_cast<char **>(&end));
                    if (start == end || errno) {
                        error = e_illegaltruss;
                        return 0;
                    }
                } else if (key == "Truss Height") {
                    errno = 0;
                    const char *start, *end;
                    start = &line.c_str()[equal_pos+1];
                    truss_height = strtod(start, const_cast<char **>(&end));
                    if (start == end || errno) {
                        error = e_illegaltruss;
                        return 0;
                    }
                } else if (key == "Left Overhang") {
                    errno = 0;
                    const char *start, *end;
                    start = &line.c_str()[equal_pos+1];
                    left_overhang = strtod(start, const_cast<char **>(&end));
                    if (start == end || errno) {
                        error = e_illegaltruss;
                        return 0;
                    }
                } else if (key == "Right Overhang") {
                    errno = 0;
                    const char *start, *end;
                    start = &line.c_str()[equal_pos+1];
                    right_overhang = strtod(start, const_cast<char **>(&end));
                    if (start == end || errno) {
                        error = e_illegaltruss;
                        return 0;
                    }
                }
                break;
            }
        }
    } while (in.good());
    
    if (span == 0) {
        error = e_illegaltruss;
        return 0;
    }
    
    if (left_overhang || right_overhang) {
        // Left and right overhang are sloping parts of the triangle, but we
        // need the amount to add to the width.
        if (right_overhang > left_overhang)
            left_overhang = right_overhang;
        
        // width = sqrt(overhang**2 / (1+slope**2))
        span += 2 * sqrt(left_overhang * left_overhang / (1 + ts * ts / 144.0));
    }
    
    return force(tt);
}

Truss *TREReader::force(truss_type override_type) {
    switch (override_type) {
    default: // tt_invalid
        error = e_unknowntruss;
        return 0;
        
    case tt_common:
        if (span == 0 || ts == 0) {
            error = e_illegaltruss;
            return 0;
        }
        // rise/run = height/(span/2)
        // rise/run = 2*height/span
        // ts/12 = 2*height/span
        // ts/24 = height/span
        // span*ts/24 = height
        return new SimpleTruss(span, (span*ts)/24.0f);
    
    case tt_monopitch:
        if (span == 0 || truss_height == 0) {
            error = e_illegaltruss;
            return 0;
        }
        return new FloorTruss(span, truss_height);
    
    case tt_floor:
        if (span == 0) {
            error = e_illegaltruss;
            return 0;
        }
        return new FloorTruss(span);
    }
}
