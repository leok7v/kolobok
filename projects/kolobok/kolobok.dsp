# Microsoft Developer Studio Project File - Name="kolobok" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=kolobok - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "kolobok.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "kolobok.mak" CFG="kolobok - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "kolobok - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "kolobok - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "kolobok - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\tmp\Release\kolobok"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W4 /GR /O1 /I "..\..\..\externals\MicrosoftSDK\include" /I "." /I "..\..\inc" /I "..\..\externals\MicrosoftSDK\include" /I "..\..\externals\jpeg-6b" /I "..\..\externals\libpng-1.2.8" /I "..\..\externals\zlib1.2.2" /I "..\..\externals\sqlite-3.2.8" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D _WIN32_WINNT=0x501 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 libcmt.lib kernel32.lib user32.lib gdi32.lib advapi32.lib comctl32.lib comdlg32.lib shell32.lib MSImg32.Lib rpcrt4.lib version.lib uuid.lib ole32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /libpath:"..\..\lib" /libpath:"..\..\externals\MicrosoftSDK\lib" /ignore:4089

!ELSEIF  "$(CFG)" == "kolobok - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\tmp\Debug\kolobok"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W4 /GR /Z7 /Od /I "." /I "..\..\inc" /I "..\..\externals\MicrosoftSDK\include" /I "..\..\externals\jpeg-6b" /I "..\..\externals\libpng-1.2.8" /I "..\..\externals\zlib1.2.2" /I "..\..\externals\sqlite-3.2.8" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D _WIN32_WINNT=0x501 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\externals\MicrosoftSDK\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"..\..\bin\kolobok-g.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcmtd.lib kernel32.lib user32.lib gdi32.lib advapi32.lib comctl32.lib comdlg32.lib shell32.lib MSImg32.Lib rpcrt4.lib version.lib uuid.lib ole32.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib /out:"..\..\bin\kolobok-g.exe" /libpath:"..\..\lib" /libpath:"..\..\externals\MicrosoftSDK\lib"

!ENDIF 

# Begin Target

# Name "kolobok - Win32 Release"
# Name "kolobok - Win32 Debug"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Group "impl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\inc\impl\ApplicationTester.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\impl\DnD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\impl\Image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\impl\MessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\impl\ObjectPipe.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\impl\String.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\impl\Window.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\inc\tiny.cpp
# End Source File
# Begin Source File

SOURCE=..\..\inc\tiny.h
# End Source File
# End Group
# Begin Group "Application"

# PROP Default_Filter "*.cpp,*.h"
# Begin Source File

SOURCE=..\..\src\kolobok\Application.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\BorderLayout.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\BorderLayout.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Controls.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Controls.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Database.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Database.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\ExifReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\ExifReader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\FolderIterator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Frame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Frame.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Help.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Help.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Log.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\MetadataReader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\MetadataReader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Panel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Panel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Skin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\Skin.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\svnrev.h.stub

!IF  "$(CFG)" == "kolobok - Win32 Release"

# Begin Custom Build - Generating "svnrev.h"
InputPath=..\..\src\kolobok\svnrev.h.stub

"..\..\src\kolobok\svnrev.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\tools\svnrev.bat $(InputPath) ..\..\src\kolobok\svnrev.h

# End Custom Build

!ELSEIF  "$(CFG)" == "kolobok - Win32 Debug"

# Begin Custom Build - Generating "svnrev.h"
InputPath=..\..\src\kolobok\svnrev.h.stub

"..\..\src\kolobok\svnrev.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\tools\svnrev.bat $(InputPath) ..\..\src\kolobok\svnrev.h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\ToolTips.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\ToolTips.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*.rc;*.ico;*.png;*.xml'*.ver"
# Begin Source File

SOURCE=..\..\src\kolobok\resources\Application.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\resources\Application.inc.ver
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\resources\Application.rc

!IF  "$(CFG)" == "kolobok - Win32 Release"

# ADD BASE RSC /l 0x409 /i "\xepec.com\projects\kolobok\src\kolobok\resources"
# ADD RSC /l 0x409 /i "\xepec.com\projects\kolobok\src\kolobok\resources"

!ELSEIF  "$(CFG)" == "kolobok - Win32 Debug"

# ADD BASE RSC /l 0x409 /i "\xepec.com\projects\kolobok\src\kolobok\resources"
# SUBTRACT BASE RSC /i "..\..\..\externals\MicrosoftSDK\include"
# ADD RSC /l 0x409 /i "\xepec.com\projects\kolobok\src\kolobok\resources"
# SUBTRACT RSC /i "..\..\..\externals\MicrosoftSDK\include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\resources\skin.png
# End Source File
# Begin Source File

SOURCE=..\..\src\kolobok\resources\skin.xml
# End Source File
# End Group
# Begin Group "external"

# PROP Default_Filter ""
# Begin Group "jpeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcapimin.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcapistd.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jccoefct.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jccolor.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcdctmgr.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jchuff.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcinit.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcmainct.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcmarker.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcmaster.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcomapi.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcparam.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcphuff.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcprepct.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jcsample.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jctrans.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdapimin.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdapistd.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdatadst.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdatasrc.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdcoefct.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdcolor.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jddctmgr.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdhuff.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdinput.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdmainct.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdmarker.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdmaster.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdmerge.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdphuff.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdpostct.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdsample.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jdtrans.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jerror.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jfdctflt.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jfdctfst.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jfdctint.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jidctflt.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jidctfst.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jidctint.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jidctred.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jmemansi.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jmemmgr.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jquant1.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jquant2.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\jutils.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\jpeg-6b\transupp.c"
# ADD CPP /W3
# End Source File
# End Group
# Begin Group "png"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\png.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngerror.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pnggccrd.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngget.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngmem.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngpread.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngread.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngrio.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngrtran.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngrutil.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngset.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngtrans.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngvcrd.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngwio.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngwrite.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngwtran.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\libpng-1.2.8\pngwutil.c"
# ADD CPP /W3
# End Source File
# End Group
# Begin Group "sqlite"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\alter.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\analyze.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\attach.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\auth.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\btree.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\build.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\callback.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\complete.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\date.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\delete.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\expr.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\func.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\hash.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\insert.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\legacy.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\main.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\opcodes.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\os_unix.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\os_win.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\pager.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\parse.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\pragma.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\prepare.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\printf.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\random.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\select.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\table.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\tokenize.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\trigger.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\update.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\utf.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\util.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\vacuum.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\vdbe.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\vdbeapi.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\vdbeaux.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\vdbefifo.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\vdbemem.c"
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE="..\..\externals\sqlite-3.2.8\where.c"
# ADD CPP /W3
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\adler32.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\compress.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\crc32.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\deflate.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\gzio.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\infback.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\inffast.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\inflate.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\inftrees.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\trees.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\uncompr.c
# ADD CPP /W3
# End Source File
# Begin Source File

SOURCE=..\..\externals\zlib1.2.2\zutil.c
# ADD CPP /W3
# End Source File
# End Group
# End Group
# End Target
# End Project
