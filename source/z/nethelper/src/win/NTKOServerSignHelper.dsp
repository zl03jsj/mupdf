# Microsoft Developer Studio Project File - Name="NTKOServerSignHelper" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=NTKOServerSignHelper - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NTKOServerSignHelper.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NTKOServerSignHelper.mak" CFG="NTKOServerSignHelper - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NTKOServerSignHelper - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "NTKOServerSignHelper - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NTKOServerSignHelper - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTKOSERVERSIGNHELPER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\00ntkocommon\inc" /I "..\..\00ntkoImage7\CxImage" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTKOSERVERSIGNHELPER_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cximage.lib ntkocmnlib.lib Wininet.lib /nologo /dll /machine:I386 /out:"OutPutDll/release/signSvrHelper.dll" /libpath:"..\..\00ntkocommon\lib\Release" /libpath:"..\..\00ntkoImage7\LibRelease"
# Begin Custom Build
TargetName=signSvrHelper
InputPath=.\OutPutDll\release\signSvrHelper.dll
InputName=signSvrHelper
SOURCE="$(InputPath)"

"OutPutDll\Release\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\ntkooledoc\VC6_PDF_RELEASE\$(TargetName).dll 
	copy $(InputPath) ..\..\ntkooledoc\RELEASE_ALL\$(TargetName).dll 
	copy $(InputPath) ..\..\ntkooledoc\TIF_NTKF_RELEASE\$(TargetName).dll 
	copy $(InputPath) ..\..\ntkooledoc\ntkoPDFviewer\Release\$(TargetName).dll 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "NTKOServerSignHelper - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTKOSERVERSIGNHELPER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\00ntkocommon\inc" /I "..\..\00ntkoImage7\CxImage" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NTKOSERVERSIGNHELPER_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cximage.lib ntkocmnlib.lib Wininet.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"OutPutDll/debug/signSvrHelper.dll" /pdbtype:sept /libpath:"..\..\00ntkocommon\lib\Debug" /libpath:"..\..\00ntkoImage7\LibDebug"
# Begin Custom Build
OutDir=.\Debug
TargetName=signSvrHelper
InputPath=.\OutPutDll\debug\signSvrHelper.dll
InputName=signSvrHelper
SOURCE="$(InputPath)"

"$(OutDir)\$(InputName).txt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\ntkooledoc\VC6_PDF_Debug\$(TargetName).dll 
	copy $(InputPath) ..\..\ntkooledoc\Debug_ALL\$(TargetName).dll 
	copy $(InputPath) ..\..\ntkooledoc\TIF_NTKF_Debug\$(TargetName).dll 
	copy $(InputPath) ..\..\ntkooledoc\ntkoPDFviewer\Debug\$(TargetName).dll 
	copy $(InputPath) ..\..\\ntkosecsign\ntkoHTMLSignHelper\ServerDebug\$(TargetName).dll 
	echo outstring 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "NTKOServerSignHelper - Win32 Release"
# Name "NTKOServerSignHelper - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\base64.cpp
# End Source File
# Begin Source File

SOURCE=.\NTKOServerSignHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\NTKOSignHelper.Def
# End Source File
# Begin Source File

SOURCE=.\NTKOSignHelperDll.cpp
# End Source File
# Begin Source File

SOURCE=.\NTKOUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\base64.h
# End Source File
# Begin Source File

SOURCE=.\NTKOMUIStrs.h
# End Source File
# Begin Source File

SOURCE=.\NTKOServerSignHelper.h
# End Source File
# Begin Source File

SOURCE=.\NTKOSignHelperDll.h
# End Source File
# Begin Source File

SOURCE=.\NTKOUtil.h
# End Source File
# Begin Source File

SOURCE=.\Res\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Res\cursorpen.cur
# End Source File
# Begin Source File

SOURCE=.\Res\hsmuistrs.bin
# End Source File
# Begin Source File

SOURCE=.\Res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00005.ico
# End Source File
# Begin Source File

SOURCE=.\Res\ico00010.ico
# End Source File
# Begin Source File

SOURCE=.\Res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Res\iconwarn.ico
# End Source File
# Begin Source File

SOURCE=.\Res\idok.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\ntkohandsign.ico
# End Source File
# Begin Source File

SOURCE=.\Res\signhelper.rc
# End Source File
# End Group
# Begin Group "mui txt"

# PROP Default_Filter "txt"
# Begin Source File

SOURCE=.\Res\hsmuistrs.txt

!IF  "$(CFG)" == "NTKOServerSignHelper - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - NTKO-Compiling $(InputPath) to .\Res\$(InputName).bin...
InputPath=.\Res\hsmuistrs.txt
InputName=hsmuistrs

".\Res\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	E:\00ntkocommon\ntkomuipt\Release\ntkomuipt.exe /i $(InputPath) /o .\Res\$(InputName).bin

# End Custom Build

!ELSEIF  "$(CFG)" == "NTKOServerSignHelper - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - NTKO-Compiling $(InputPath) to .\Res\$(InputName).bin...
InputPath=.\Res\hsmuistrs.txt
InputName=hsmuistrs

".\Res\$(InputName).bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	E:\00ntkocommon\ntkomuipt\Release\ntkomuipt.exe /i $(InputPath) /o .\Res\$(InputName).bin

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Res\signhelper.manifest
# End Source File
# End Target
# End Project
