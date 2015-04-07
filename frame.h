#ifndef frame_h
#define frame_h

#include <wx/frame.h>
#include <wx/menu.h>

class WeekView;

class ScheduleFrame : public wxFrame {
public:
    ScheduleFrame();
    ~ScheduleFrame();
    
    enum {
        kFileNew = 1,
        kFileOpen,
        kFileSave,
        kFileSaveAs,
        kFileImportTRE,
        kFilePrint,
        //kFilePrintPreview,
        kFilePageSetup,
        kFileExit,
        
        kEditCut,
        kEditCopy,
        kEditPaste,
        kEditDelete,
        
        kDatabaseImport,
        kDatabaseManage,
        kDatabaseSetup
    };
    
    void onMenu(wxCommandEvent &event);
    void onClose(wxCloseEvent &event);
    
    void setModifiedState(bool mod = true);
    bool openFile();
    bool openFile(const wxString &new_filename);
    bool saveFile(bool saveAs);
    
    void importTRE(int day = -1);

private:
    WeekView *weekView;
    wxString filename;
    bool modified;
};

#endif // frame_h
