#ifndef app_h
#define app_h

#include <wx/app.h>
#include <wx/event.h>

class ScheduleApp : public wxApp {
public:
    virtual bool OnInit();
    virtual int OnExit();
    
    virtual void OnInitCmdLine(wxCmdLineParser &parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
};

DECLARE_APP(ScheduleApp);

#endif // app_h
