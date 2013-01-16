;NSIS script for CSkinTools installation for windows
; external variables should passed to compiler:
;    VERSION : plugin version
;    OUTFILE : installer output file
;    MAYAVERSION
;    MAYAPLATFORM
;    MAYA_MODULE_CONTENTS: folder of maya module contents
;    LICENSE: license in txt format
;--------------------------------

OutFile "${OUTFILE}"

;!define VERSION 'NOVERSION'
;!define MAYAVERSION '2011'
;!define MAYAPLATFORM "64"

!if ${MAYAPLATFORM} != "64"
  !undef MAYAPLATFORM
  !define MAYAPLATFORM "32"
!endif


Var MAYA_INSTALL_LOCATION


!if "${MAYAPLATFORM}" == '64'
  !define MAYAPLATFORM_NUMBER 64
  !define MAYADOCUMENTSFOLDER "$DOCUMENTS\maya\${MAYAVERSION}-x64\"
!else
  !define MAYAPLATFORM_NUMBER 32
  !define MAYADOCUMENTSFOLDER "$DOCUMENTS\maya\${MAYAVERSION}\"
!endif

!define MAYA_READABLE 'Maya${MAYAVERSION}/${MAYAPLATFORM}bit'

;--------------------------------
;Configuration

SetCompressor /SOLID lzma
;InstType "Full"
InstallDir "$PROGRAMFILES\ngSkinTools\Maya${MAYAVERSION}-${MAYAPLATFORM}bit"
RequestExecutionLevel admin


!include "MUI2.nsh"
!include "LogicLib.nsh"

;Name and file
Name "ngSkinTools-${VERSION}-${MAYA_READABLE}"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "${LICENSE}"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Function .onInit

    ; find maya install path
    SetRegView ${MAYAPLATFORM_NUMBER}
    ReadRegStr $MAYA_INSTALL_LOCATION HKLM SOFTWARE\Autodesk\Maya\${MAYAVERSION}\Setup\InstallPath "MAYA_INSTALL_LOCATION"
    IfErrors 0 +3
        MessageBox MB_OK "Required version of Maya (${MAYA_READABLE}) is not installed"
        Abort 
    Nop      
        
FunctionEnd

Section "Plugin" PluginSection
    SectionIn 1
    SectionIn RO

    
    ; read maya install path
    DetailPrint "Maya ${MAYAVERSION} found at: $MAYA_INSTALL_LOCATION"
      
    ; add module description to maya
    !define MODULE_NAME_FILE ngskintools.txt
    SetOutPath "$MAYA_INSTALL_LOCATION\modules"
    DetailPrint "Registering module in Maya"
    FileOpen $0 ngskintools.txt w
    FileWrite $0 "+ ngSkinTools ${VERSION} $INSTDIR"
    FileClose $0
      
    SetOutPath "$INSTDIR"
    File /r /x "${MAYA_MODULE_CONTENTS}\install.py" "${MAYA_MODULE_CONTENTS}\*.*"
      
    ;Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Shelf buttons" ShelfButtonsSection
    SectionIn 1

    SetOutPath "${MAYADOCUMENTSFOLDER}\prefs\shelves"
    File "..\mel\shelf_ngSkinTools.mel"
SectionEnd 

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_PluginSection ${LANG_ENGLISH} "Plugin Files"
  LangString DESC_ShelfButtonsSection ${LANG_ENGLISH} "Maya shelf buttons for launching the tool"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${PluginSection} $(DESC_PluginSection)
    !insertmacro MUI_DESCRIPTION_TEXT ${ShelfButtonsSection} $(DESC_ShelfButtonsSection)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
    Delete "${MAYADOCUMENTSFOLDER}\prefs\shelves\shelf_ngSkinTools.mel" 
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR"
SectionEnd