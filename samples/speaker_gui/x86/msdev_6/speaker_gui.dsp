# Microsoft Developer Studio Project File - Name="speaker_gui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=speaker_gui - Win32  Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "speaker_gui.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "speaker_gui.mak" CFG="speaker_gui - Win32  Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "speaker_gui - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "speaker_gui - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
# PROP Target_Last_Scanned "speaker_gui - Win32 Debug"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "speaker_gui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP -Di386 -DWD_DRIVER_NAME_CHANGE /nologo /O2 /D "NDEBUG" /W3 /EHsc /D "WIN32"  /D "_WINDOWS"  /c
# ADD CPP -Di386 -DWD_DRIVER_NAME_CHANGE  /nologo /O2 /D "NDEBUG" /W3 /EHsc  /D "WIN32" /I "../../../..//include" /I "../../../../"   /D "_WINDOWS"  /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/speaker.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wdapi920.lib /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:windows /machine:I386
# ADD LINK32 wdapi920.lib /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "speaker_gui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP -Di386 -DWD_DRIVER_NAME_CHANGE /nologo /Gm /Zi /Od /D "_DEBUG" /W3 /EHsc /D "WIN32"  /D "_WINDOWS"  /c
# ADD CPP -Di386 -DWD_DRIVER_NAME_CHANGE  /nologo /Gm /Zi /Od /D "_DEBUG" /W3 /EHsc  /D "WIN32" /I "../../../..//include" /I "../../../../"   /D "_WINDOWS"  /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/speaker.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wdapi920.lib /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 wdapi920.lib /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "speaker_gui - Win32 Release"
# Name "speaker_gui - Win32 Debug"

################################################################################
# Begin Source File

SOURCE=..\..\../speaker/speaker_lib.c

# End Source File

################################################################################
# Begin Source File

SOURCE=..\..\speaker_gui.c

# End Source File

################################################################################
# Begin Source File

SOURCE=..\..\speaker.rc

# End Source File

# End Target
# End Project

