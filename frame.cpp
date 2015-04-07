#include "frame.h"
#include "weekview.h"
#include "dayview.h"
#include "trussthumbview.h"
#include "trereader.h"
#include "truss.h"
#include "daypickerdlg.h"
#include "treatasdlg.h"
#include "printing.h"
#include "dbdialog.h"
#include "database.h"

#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/cmndata.h>
#include <wx/printdlg.h>

#include <cassert>

ScheduleFrame::ScheduleFrame() :
    wxFrame(0, wxID_ANY, wxT("Visual Schedule"), wxDefaultPosition,
        wxSize(1020, 600), wxDEFAULT_FRAME_STYLE), modified(false) {
                
    wxMenuBar *mbar = new wxMenuBar();
    
    wxMenu *menu_file = new wxMenu();
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFileNew,
        wxT("&New Schedule...\tCtrl+N"),
        wxT("Create a new week schedule in a new window"),
        wxITEM_NORMAL
    ));
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFileOpen,
        wxT("&Open...\tCtrl+O"),
        wxT("Open a schedule file"),
        wxITEM_NORMAL
    ));
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFileSave,
        wxT("&Save\tCtrl+S"),
        wxT("Save the current schedule"),
        wxITEM_NORMAL
    ));
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFileSaveAs,
        wxT("Save &As...\tShift+Ctrl+S"),
        wxT("Save the current schedule as a new file"),
        wxITEM_NORMAL
    ));
    
    menu_file->AppendSeparator();
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFileImportTRE,
        wxT("&Import .TRE file...\tCtrl+T"),
        wxT("Import a truss drawing from a MiTek .TRE file"),
        wxITEM_NORMAL
    ));
        
    menu_file->AppendSeparator();
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFilePrint,
        wxT("&Print...\tCtrl+P"),
        wxT("Print the week's schedule"),
        wxITEM_NORMAL
    ));
    
    /*menu_file->Append(new wxMenuItem(
        menu_file,
        kFilePrintPreview,
        wxT("Print Preview..."),
        wxT("Look at what it's gonna look like when you actually print it"),
        wxITEM_NORMAL
    ));*/
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFilePageSetup,
        wxT("Page Setup..."),
        wxT("Does anyone even read these things???"),
        wxITEM_NORMAL
    ));
    
    menu_file->AppendSeparator();
    
    menu_file->Append(new wxMenuItem(
        menu_file,
        kFileExit,
        wxT("E&xit\tAlt+F4"),
        wxT("Quit the program"),
        wxITEM_NORMAL
    ));
    
    mbar->Append(menu_file, wxT("&File"));
    
    //-----
    
    wxMenu *menu_edit = new wxMenu();
    
    menu_edit->Append(new wxMenuItem(
        menu_edit,
        kEditCut,
        wxT("Cu&t\tCtrl+X"),
        wxT("Cut the selected truss drawing"),
        wxITEM_NORMAL
    ));
    
    menu_edit->Append(new wxMenuItem(
        menu_edit,
        kEditCopy,
        wxT("&Copy\tCtrl+C"),
        wxT("Copy the selected truss drawing"),
        wxITEM_NORMAL
    ));
    
    menu_edit->Append(new wxMenuItem(
        menu_edit,
        kEditPaste,
        wxT("&Paste\tCtrl+V"),
        wxT("Paste the selected truss drawing"),
        wxITEM_NORMAL
    ));
    
    menu_edit->Append(new wxMenuItem(
        menu_edit,
        kEditDelete,
        wxT("&Delete"),
        wxT("Delete the selected truss drawing"),
        wxITEM_NORMAL
    ));
    
    mbar->Append(menu_edit, wxT("&Edit"));
    
    //-----
    
    wxMenu *menu_database = new wxMenu();
    
    menu_database->Append(new wxMenuItem(
        menu_database,
        kDatabaseImport,
        wxT("&Import truss...\tCtrl+I"),
        wxT("Import a truss from the database"),
        wxITEM_NORMAL
    ));
    
    menu_database->Append(new wxMenuItem(
        menu_database,
        kDatabaseManage,
        wxT("&Manage stored trusses...\tCtrl+M"),
        wxT("Move, rename, or delete stored trusses"),
        wxITEM_NORMAL
    ));
    
    menu_database->AppendSeparator();
    
    menu_database->Append(new wxMenuItem(
        menu_database,
        kDatabaseSetup,
        wxT("&Setup..."),
        wxT("Connect to or create a database"),
        wxITEM_NORMAL
    ));
    
    mbar->Append(menu_database, wxT("&Database"));
    
    //-----
        
    SetMenuBar(mbar);
    
    //CreateStatusBar(1, wxST_SIZEGRIP, wxID_ANY);
    
    weekView = new WeekView(this);
    
    Bind(wxEVT_COMMAND_MENU_SELECTED, &ScheduleFrame::onMenu, this);
    Bind(wxEVT_CLOSE_WINDOW, &ScheduleFrame::onClose, this);
    
#ifdef __WXMSW__
    SetIcon(wxICON(frame_icon));
#endif

    Centre(wxBOTH);
    Show();
}

ScheduleFrame::~ScheduleFrame() {
}

// From app.cpp, for printing.
extern wxPrintData *print_data;
extern wxPageSetupDialogData *page_setup_dialog_data;

void ScheduleFrame::onMenu(wxCommandEvent &event) {
    int id = event.GetId();
    switch (id) {
    case kFileNew:
        (new ScheduleFrame())->Show(true);
        break;
    
    case kFileOpen:
        openFile();
        break;
        
    case kFileSave:
        saveFile(false);
        break;
    
    case kFileSaveAs:
        saveFile(true);
        break;
    
    case kFilePrint: {
        TrussPrintout *p = weekView->printout();
        // It could be null, if there is nothing to print.
        if (!p)
            break;
        
        wxPrintDialogData data(*print_data);
        wxPrinter printer(&data);
        bool success = printer.Print(this, p, true);
        delete p;
    }
        break;
        
    /*case kFilePrintPreview: {
        // This seems like a poor design, but I guess we have to do it that way.
        TrussPrintout *p1 = weekView->printout();
        TrussPrintout *p2 = weekView->printout();
        if (!p1)
            break;
        
        wxPrintDialogData data(*print_data);
        wxPrintPreview *preview = new wxPrintPreview(p1, p2, &data);
        if (!preview->IsOk()) {
            delete preview;
            break;
        }
        
        wxPreviewFrame *pf =
            new wxPreviewFrame(preview, this, wxT("Print preview"));
        pf->Centre(wxBOTH);
        //pf->InitializeWithModality(wxPreviewFrame_AppModal);
        pf->Show(true);
    }
        break;*/
        
    case kFilePageSetup: {
        *page_setup_dialog_data = *print_data;
        
        wxPageSetupDialog dlg(this, page_setup_dialog_data);
        if (dlg.ShowModal() != wxID_CANCEL) {
            *page_setup_dialog_data = dlg.GetPageSetupDialogData();
            *print_data = page_setup_dialog_data->GetPrintData();
        }
    }
        break;
    
    case kFileExit:
        Close();
        break;
        
    case kFileImportTRE:
        importTRE();
        break;
            
    case kEditCut: {
        TrussThumbView *ttv = weekView->getSelection();
        if (!ttv)
            break;
        
        ttv->copyToClipboard();
        weekView->deselect();
        wxWindow *parent = ttv->GetParent();
        ttv->Destroy();
        parent->Layout();
        parent->FitInside();
    }
        break;
    
    case kEditCopy: {
        TrussThumbView *ttv = weekView->getSelection();
        if (!ttv)
            break;
        
        ttv->copyToClipboard();
    }
        break;
    
    case kEditPaste:
        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                if (TrussThumbView *ttv = weekView->getSelection()) {
                    wxTextDataObject text;
                    wxTheClipboard->GetData(text);
                    Truss *truss = Truss::unpack(text.GetText().c_str());
                    if (truss) {
                        ((DayView *)ttv->GetParent())->insertAfter(ttv, truss);
                        setModifiedState();
                    }
                }
            }
            
            wxTheClipboard->Close();
        }
        break;
    
    case kEditDelete:
        if (TrussThumbView *ttv = weekView->getSelection()) {
            weekView->deselect();
            wxWindow *parent = ttv->GetParent();
            ttv->Destroy();
            parent->Layout();
            parent->FitInside();
        }
        break;
    
    case kDatabaseImport:
    case kDatabaseManage: {
        DbDialog dlg(this, id == kDatabaseManage);
        if (dlg.ShowModal() == wxID_CANCEL)
            break;
        
        // If we opened the dialog in manage mode, it will take care of
        // whatever needs to be done.
        if (id == kDatabaseManage)
            break;

        Truss *truss;
        if (!(truss = dlg.getTruss())) {
            wxMessageBox(wxT("Couldn't import truss!"), wxT("Error"));
            break;
        }

        // Which day do we add it into?
        int day = -1;
        DayPickerDlg daypicker(this);
        daypicker.ShowModal();
        day = daypicker.getDay();
        if (day == -1) {
            delete truss;
            break;
        }

        // Import the truss.
        weekView->putTruss(day, truss);
    }
        break;

    case kDatabaseSetup: {
        DbSetupDlg dlg(this);
        if (dlg.ShowModal() == wxID_CANCEL)
            break;

        if (dlg.hasDb()) {
            if (!db_set_filename(dlg.getFilename()))
                wxMessageBox(wxT("Couldn't store database name!"),
                        wxT("Error!"));
        } else {
            if (!db_create(dlg.getFilename()))
                wxMessageBox(wxT("Couldn't create database!"),
                        wxT("Error!"));
        }
    }
        break;

    default:
        event.Skip();
    }
}

void ScheduleFrame::onClose(wxCloseEvent &event) {
    if (!modified) {
        event.Skip();
        return;
    }

    int flags = wxYES_NO;
    if (event.CanVeto())
        flags |= wxCANCEL;

    int result = wxMessageBox(wxT("The current schedule has not been saved. ")
            wxT("Would you like to save it?"),
        wxT("Save changes?"), flags, this);
        
    if (result == wxCANCEL) {
        event.Veto();
        return;
    }
    
    if (result == wxNO) {
        event.Skip();
        return;
    }
    
    // Only close if we could actually save the file.
    if (saveFile(false) || !event.CanVeto())
        event.Skip();
    else
        event.Veto();
}

// Note - on most platforms, a * in the title shows it's been modified. On OSX,
// you put a dot in the close button and leave the title alone.
void ScheduleFrame::setModifiedState(bool mod) {
    modified = mod;
    
    wxString newtitle;
    if (filename.empty()) {
#ifndef __WXOSX__
        if (mod)
            newtitle = wxT("Visual Schedule *");
        else
#endif
            newtitle = wxT("Visual Schedule");
    } else {
        newtitle = filename;
#ifndef __WXOSX__
        if (mod)
            newtitle += wxT(" * - Visual Schedule");
        else
#endif
            newtitle += wxT(" - Visual Schedule");
    }
        
    SetTitle(newtitle);
    
#ifdef __WXOSX__
    // Put the dot in the window close button.
    OSXSetModified(mod);
#endif
}

bool ScheduleFrame::openFile() {
    wxFileDialog opendlg(this, wxT("Open schedule file"), wxT(""), wxT(""),
        wxT("Schedule files|*.tsched|All files|*.*"),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        
    if (opendlg.ShowModal() == wxID_CANCEL)
        return false;
    
    // If this frame is empty and has no open file, open the file here.
    // Otherwise open it in a new frame.
    if (filename.empty() && weekView->empty())
        return openFile(opendlg.GetPath());
    else {
        ScheduleFrame *new_frame = new ScheduleFrame();
        if (!new_frame->openFile(opendlg.GetPath())) {
            new_frame->Destroy();
            return false;
        }
        
        new_frame->Show(true);
        return true;
    }
}

bool ScheduleFrame::openFile(const wxString &new_filename) {
    if (!weekView->readFrom(new_filename.c_str()))
        return false;
    
    filename = new_filename;
    setModifiedState(false);
    return true;
}

bool ScheduleFrame::saveFile(bool saveAs) {
    if (saveAs) {
        wxFileDialog savedlg(this, wxT("Save schedule file"), wxT(""), wxT(""),
            wxT("Schedule file|*.tsched|All files|*.*"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        
        if (savedlg.ShowModal() == wxID_CANCEL)
            return false;
    
        wxString new_filename = savedlg.GetPath();
        if (weekView->writeTo(new_filename.c_str())) {
            filename = new_filename;
            setModifiedState(false);
            return true;
        }
        
        wxMessageBox(wxT("I wasn't able to save that file. ")
            wxT("Maybe it's protected?"), wxT("File writin' error"),
            wxOK, this);
            
        return false;
    }
    
    if (!weekView->writeTo(filename.c_str())) {
        int result = wxMessageBox(wxT("I wasn't able to save that file. ")
            wxT("Would you like to pick a new one?"), wxT("File writin' error"),
            wxYES_NO, this);
        
        if (result == wxYES)
            return saveFile(true);
        
        return false;
    }
    
    setModifiedState(false);
    return true;
}

void ScheduleFrame::importTRE(int day) {
    wxFileDialog opendlg(this, wxT("Import .TRE file"), wxT(""), wxT(""),
        wxT("MiTek .TRE files|*.tre|All files|*.*"),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (opendlg.ShowModal() == wxID_CANCEL)
        return;
    
    TREReader tre_reader(opendlg.GetPath().c_str());
    Truss *truss = tre_reader.getTruss();
    if (!truss) {
        wxString error_msg;
        TREReader::ErrorCode ec = tre_reader.getError();
        switch (ec) {
        case TREReader::e_ok:
            assert(0 && "the .TRE reading error wasn't diagnosed");
            error_msg = wxT("Unknown error.");
            break;
        
        case TREReader::e_uninitialized:
            assert(0 && "Should only happen when getTruss hasn't been called");
            error_msg = wxT("Unknown error.");
            break;
            
        case TREReader::e_cantopen:
            error_msg = wxT("Couldn't open the .TRE file.");
            break;
            
        case TREReader::e_invalidformat:
            error_msg = wxT("The file contains no truss information.");
            break;
            
        case TREReader::e_illegaltruss:
            error_msg = wxT("Invalid parameters found.");
            break;
            
        case TREReader::e_unknowntruss: {
            // If force() returns null, this message will be shown.
            error_msg = wxT("The truss could not be imported as ")
                wxT("the specified type.");
            TreatAsDlg tad(this, tre_reader.getTrussType());
            if (tad.ShowModal() == wxID_CANCEL)
                return;
            truss = tre_reader.force(tad.getSelection());
        }
            break;
        }
        
        // If we got an unknown truss and forced it to a certain type,
        // it could be valid now.
        if (!truss) {
            wxMessageBox(error_msg, wxT("Error reading truss info"), wxOK,
                this);
            return;
        }
    }
    
    // If day was not specified, ask for it.
    if (day == -1) {
        DayPickerDlg daypicker(this);
        daypicker.ShowModal();
        day = daypicker.getDay();
        if (day == -1) {
            delete truss;
            return;
        }
    }
    weekView->putTruss(day, truss);

    int import_to_db = wxMessageBox(wxT("Would you like to import this truss ")
        wxT("into the database?"), wxT("Confirm"), wxYES_NO);
    if (import_to_db == wxYES)
        db_try_import(this, truss);
}
