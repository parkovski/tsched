#ifndef trereader_h
#define trereader_h

#include <fstream>
#include <string>

class Truss;

class TREReader {
public:
    enum truss_type {
        tt_invalid,         // no known truss type
        tt_common,          // common truss (triangle)
        tt_monopitch,       // looks kind of like this: |/|.
        tt_floor            // box, actually a pack of 13 floor trusses.
    };

    enum ErrorCode {
        e_ok,               // no error
        e_uninitialized,    // nothing has been read yet
        e_cantopen,         // file couldn't be opened
        e_invalidformat,    // no truss information found in the file
        e_illegaltruss,     // invalid parameters (<= 0, e.g.) found
        e_unknowntruss      // type of truss not recognized
    };

private:
    std::ifstream in;
    ErrorCode error;
    
    // parameters to save between calls to getTruss and force.
    float span, ts, truss_height;
    std::string type;
    
public:
    TREReader(const char *filename);
    ~TREReader();

    Truss *getTruss();
    Truss *force(truss_type override_type);
    const std::string &getTrussType() { return type; }
    
    ErrorCode getError() const { return error; }
};

#endif // trereader_h
