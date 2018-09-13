# Microsoft Developer Studio Project File - Name="i8253" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=i8253 - Win32  Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "i8253.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "i8253.mak" CFG="i8253 - Win32  Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "i8253 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "i8253 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
# PROP Target_Last_Scanned "i8253 - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "i8253 - Win32 Release"

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
# ADD BASE CPP -Di386 -DWD_DRIVER_NAME_CHANGE /nologo /O2 /D "NDEBUG" /W3 /EHsc /D "WIN32"  /D "_CONSOLE" /D "_MBCS" /FD /c
# ADD CPP -Di386 -DWD_DRIVER_NAME_CHANGE  /nologo /O2 /D "NDEBUG" /W3 /EHsc  /D "WIN32" /I "../../../..//include" /I "../../../../"   /D "_CONSOLE" /D "_MBCS" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:console /machine:I386 /pdbtype:sept
# ADD LINK32  /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:console /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "i8253 - Win32 Debug"

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
# ADD BASE CPP -Di386 -DWD_DRIVER_NAME_CHANGE /nologo /Gm /Zi /Od /D "_DEBUG" /W3 /EHsc /D "WIN32"  /D "_CONSOLE" /D "_MBCS" /FD /c
# ADD CPP -Di386 -DWD_DRIVER_NAME_CHANGE  /nologo /Gm /Zi /Od /D "_DEBUG" /W3 /EHsc  /D "WIN32" /I "../../../..//include" /I "../../../../"   /D "_CONSOLE" /D "_MBCS" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32  /LIBPATH:"..\..\..\..\lib\x86\" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "i8253 - Win32 Release"
# Name "i8253 - Win32 Debug"

################################################################################
# Begin Source File

SOURCE=..\..\i8253.c

# End Source File

################################################################################
# Begin Source File

SOURCE=..\..\../../src/wdapi/windrvr.c

# End Source File

################################################################################
# Begin Source File

SOURCE=..\..\../../src/wdapi/utils.c

# End Source File

# End Target
# End Project

