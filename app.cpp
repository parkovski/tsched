#include "app.h"
#include "frame.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/cmndata.h>
#include <wx/snglinst.h>
#include <wx/cmdline.h>
#include <wx/arrstr.h>
#include <wx/stdpaths.h>

IMPLEMENT_APP(ScheduleApp);

// Needed by bed view and thumb view.
extern wxBitmap *excl_icon, *excl_icon_small, *full_icon, *oversize_icon,
    *notes_icon;
wxBitmap *excl_icon, *excl_icon_small, *full_icon, *oversize_icon, *notes_icon;

extern wxPrintData *print_data;
wxPrintData *print_data;
extern wxPageSetupDialogData *page_setup_dialog_data;
wxPageSetupDialogData *page_setup_dialog_data;

// Used between OnCmdLineParsed and OnInit.
wxArrayString *cmd_args;

bool ScheduleApp::OnInit() {
    if (!wxApp::OnInit()) {
        delete cmd_args;
        return false;
    }
    
    wxString exe_path = wxStandardPaths::Get().GetExecutablePath();
    int slash_pos = exe_path.find_last_of(wxT('/'));
#ifdef __WXMSW__
    int backslash_pos = exe_path.find_last_of(wxT('\\'));
    if (backslash_pos > slash_pos)
        slash_pos = backslash_pos;
#endif
    
    if (slash_pos != -1) {
#ifdef __WXOSX__
        exe_path = exe_path.substr(0, slash_pos + 1) + wxT("../../../../");
#else
        exe_path = exe_path.substr(0, slash_pos + 1) + wxT("../");
#endif
    }

    wxImage::AddHandler(new wxPNGHandler);
    excl_icon = new wxBitmap(exe_path + wxT("images/excl.png"),
        wxBITMAP_TYPE_PNG);
    excl_icon_small =
        new wxBitmap(exe_path + wxT("images/excl-small.png"),
        wxBITMAP_TYPE_PNG);
    full_icon = new wxBitmap(exe_path + wxT("images/fulltruck.png"),
        wxBITMAP_TYPE_PNG);
    oversize_icon =
        new wxBitmap(exe_path + wxT("images/oversize.png"), wxBITMAP_TYPE_PNG);
    notes_icon = new wxBitmap(exe_path + wxT("images/notes.png"),
        wxBITMAP_TYPE_PNG);
    
    print_data = new wxPrintData;
    print_data->SetPaperId(wxPAPER_LETTER);
    
    page_setup_dialog_data = new wxPageSetupDialogData;
    *page_setup_dialog_data = *print_data;
    
    // These are measured in millimeters.
    page_setup_dialog_data->SetMarginTopLeft(wxPoint(15, 15));
    page_setup_dialog_data->SetMarginBottomRight(wxPoint(15, 15));

    int cmd_args_count = cmd_args->GetCount();
    if (cmd_args_count > 0) {
        for (int i = 0; i < cmd_args_count; ++i) {
            ScheduleFrame *frame = new ScheduleFrame();
            if (!frame->openFile((*cmd_args)[i]))
                frame->Destroy();
            else
                frame->Show(true);
        }
    } else {
        // No args: open a frame with no loaded document.
        (new ScheduleFrame())->Show(true);
    }
    
    // Don't need this to stick around the whole time the program is running.
    delete cmd_args;
    return true;
}

int ScheduleApp::OnExit() {
    delete excl_icon;
    delete excl_icon_small;
    delete full_icon;
    delete oversize_icon;
    delete notes_icon;
    
    delete print_data;
    delete page_setup_dialog_data;
        
    return 0;
}

static const wxCmdLineEntryDesc cmd_line_desc[] = {
    { wxCMD_LINE_PARAM, 0, 0, "schedule files", wxCMD_LINE_VAL_STRING,
        wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_NONE }
};

void ScheduleApp::OnInitCmdLine(wxCmdLineParser &parser) {
    parser.SetDesc(cmd_line_desc);
    parser.SetSwitchChars(wxT("-"));
}

bool ScheduleApp::OnCmdLineParsed(wxCmdLineParser &parser) {
    cmd_args = new wxArrayString();
    for (int i = 0; i < parser.GetParamCount(); ++i) {
        cmd_args->Add(parser.GetParam(i));
    }
    
    return true;
}
