#ifndef printing_h
#define printing_h

#include "truss.h"

#include <wx/print.h>

#include <vector>
#include <string>

class Page {
    std::vector<Truss *> trusses;
    std::string header;
    // Margins:
    int mleft, mtop, mright, mbottom;
    
public:
    Page(const std::string &header, int ml, int mt, int mr, int mb)
        : header(header), mleft(ml), mtop(mt), mright(mr), mbottom(mb) { }

    void print(wxDC &dc);
    
    void add(Truss *truss) { trusses.push_back(truss); }
    
private:
    void printTruss(wxDC &dc, Truss *truss, int &top, int width);
};

//-----

// These are necessary because we have to fill the printout with trusses
// before we even know the paper size. Thanks a lot wxWidgets.

struct PrintItem {
    // true => it's a page break, otherwise it's a truss.
    virtual bool isbreak() = 0;
};

struct PageBreak : public PrintItem {
    std::string header;
    
    PageBreak(const std::string &header) : header(header) { }
    
    virtual bool isbreak() { return true; }
};

struct TrussContainer : public PrintItem {
    Truss *truss;
    
    TrussContainer(Truss *truss) : truss(truss) { }
    
    virtual bool isbreak() { return false; }
};

//-----

class TrussPrintout : public wxPrintout {
    std::vector<Page> pages;
    std::vector<PrintItem *> printitems;

public:
    TrussPrintout(const char *day, Truss *truss = 0);
    ~TrussPrintout();
    
    void addTruss(Truss *truss);
    void pageBreak(const char *header);
    
    virtual void OnPreparePrinting();
    virtual void GetPageInfo(int *minPage, int *maxPage,
                             int *pageFrom, int *pageTo);
    virtual bool HasPage(int pageNum);
    virtual bool OnPrintPage(int pageNum);
};

#endif // printing_h
