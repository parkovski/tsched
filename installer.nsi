!include "MUI2.nsh"

Name "Visual Scheduler"
OutFile "Install Scheduler.exe"

; Because we have to create a registry key (register .tsched extension)
RequestExecutionLevel admin

InstallDir "$PROGRAMFILES\Visual Scheduler"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install Scheduler" SecTest
    SetOutPath "$INSTDIR"
    
    CreateDirectory "$INSTDIR\bin"
    CreateDirectory "$INSTDIR\images"
    
    File /oname=bin\scheduler.exe windows\scheduler.exe
    File /oname=images\excl.png images\excl.png
    File /oname=images\excl-small.png images\excl-small.png
    File /oname=images\fulltruck.png images\fulltruck.png
    File /oname=images\oversize.png images\oversize.png
    File /oname=images\notes.png images\notes.png
    
    WriteRegStr HKCR ".tsched" "" "VisualScheduler.Document"
    WriteRegStr HKCR "VisualScheduler.Document\DefaultIcon" "" \
        "$INSTDIR\bin\scheduler.exe,10"
    WriteRegStr HKCR "VisualScheduler.Document\shell\open\command" "" \
        '"$INSTDIR\bin\scheduler.exe" "%1"'
    
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Add Desktop Shortcut" SecShortcut
    SetShellVarContext current
    CreateShortCut "$DESKTOP\Visual Scheduler.lnk" \
        "$INSTDIR\bin\scheduler.exe"
SectionEnd

Section "Add Start Menu Shortcut" SecStart
    CreateDirectory "$SMPROGRAMS\Visual Scheduler"
    CreateShortCut "$SMPROGRAMS\Visual Scheduler\Visual Scheduler.lnk" \
        "$INSTDIR\bin\scheduler.exe"
SectionEnd


;Language strings
LangString DESC_SecTest ${LANG_ENGLISH} "Install the program."
LangString DESC_SecShortcut ${LANG_ENGLISH} "Add a shortcut to the desktop."
LangString DESC_SecStart ${LANG_ENGLISH} "Add a shortcut to the start menu."

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecTest} $(DESC_SecTest)
!insertmacro MUI_DESCRIPTION_TEXT ${SecShortcut} $(DESC_SecShortcut)
!insertmacro MUI_DESCRIPTION_TEXT ${SecStart} $(DESC_SecStart)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"

    DeleteRegKey HKCR ".tsched"
    DeleteRegKey HKCR "VisualScheduler.Document"

    Delete "$DESKTOP\Visual Scheduler.lnk"
    Delete "$SMPROGRAMS\Visual Scheduler\*.*"
    RMDir "$SMPROGRAMS\Visual Scheduler"

    RMDir /r "$INSTDIR"
    
SectionEnd
