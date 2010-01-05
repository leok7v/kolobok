# Microsoft Developer Studio Generated NMAKE File, Based on tests.dsp
!IF "$(CFG)" == ""
CFG=tests - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tests - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tests - Win32 Release" && "$(CFG)" != "tests - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tests.mak" CFG="tests - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tests - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tests - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "tests - Win32 Release"

OUTDIR=.\..\..\bin
INTDIR=.\..\..\tmp\Release\tests
# Begin Custom Macros
OutDir=.\..\..\bin
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\tests.exe" "..\..\tmp\Release\tests\tests.bsc"

!ELSE 

ALL : "BookSmart - Win32 Release" "$(OUTDIR)\tests.exe"\
 "..\..\tmp\Release\tests\tests.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"BookSmart - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\Image.obj"
	-@erase "$(INTDIR)\Image.sbr"
	-@erase "$(INTDIR)\manifest.obj"
	-@erase "$(INTDIR)\manifest.sbr"
	-@erase "$(INTDIR)\MessageQueue.obj"
	-@erase "$(INTDIR)\MessageQueue.sbr"
	-@erase "$(INTDIR)\Str.obj"
	-@erase "$(INTDIR)\Str.sbr"
	-@erase "$(INTDIR)\test_app.obj"
	-@erase "$(INTDIR)\test_app.sbr"
	-@erase "$(INTDIR)\TesterPipe.obj"
	-@erase "$(INTDIR)\TesterPipe.sbr"
	-@erase "$(INTDIR)\tests.obj"
	-@erase "$(INTDIR)\tests.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\Window.obj"
	-@erase "$(INTDIR)\Window.sbr"
	-@erase "$(OUTDIR)\tests.exe"
	-@erase "..\..\tmp\Release\tests\tests.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W4 /GR /O1 /Ob2 /I "..\..\include" /I\
 "..\..\..\externals\MicrosoftSDK\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "UNICODE" /D _WIN32_WINNT=0x501 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=..\..\tmp\Release\tests/
CPP_SBRS=..\..\tmp\Release\tests/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"..\..\tmp\Release\tests/tests.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Image.sbr" \
	"$(INTDIR)\manifest.sbr" \
	"$(INTDIR)\MessageQueue.sbr" \
	"$(INTDIR)\Str.sbr" \
	"$(INTDIR)\test_app.sbr" \
	"$(INTDIR)\TesterPipe.sbr" \
	"$(INTDIR)\tests.sbr" \
	"$(INTDIR)\Window.sbr"

"..\..\tmp\Release\tests\tests.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=jpeg.lib png.lib zlib.lib kernel32.lib user32.lib gdi32.lib\
 advapi32.lib comctl32.lib shell32.lib MSImg32.Lib rpcrt4.lib /nologo\
 /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\tests.exe"\
 /libpath:"..\..\lib" /libpath:"..\..\..\externals\MicrosoftSDK\lib" 
LINK32_OBJS= \
	"$(INTDIR)\Image.obj" \
	"$(INTDIR)\manifest.obj" \
	"$(INTDIR)\MessageQueue.obj" \
	"$(INTDIR)\Str.obj" \
	"$(INTDIR)\test_app.obj" \
	"$(INTDIR)\TesterPipe.obj" \
	"$(INTDIR)\tests.obj" \
	"$(INTDIR)\Window.obj"

"$(OUTDIR)\tests.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tests - Win32 Debug"

OUTDIR=.\..\..\bin
INTDIR=.\..\..\tmp\Debug\tests
# Begin Custom Macros
OutDir=.\..\..\bin
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\tests-g.exe" "..\..\tmp\Debug\tests.bsc"

!ELSE 

ALL : "BookSmart - Win32 Debug" "$(OUTDIR)\tests-g.exe"\
 "..\..\tmp\Debug\tests.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"BookSmart - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\Image.obj"
	-@erase "$(INTDIR)\Image.sbr"
	-@erase "$(INTDIR)\manifest.obj"
	-@erase "$(INTDIR)\manifest.sbr"
	-@erase "$(INTDIR)\MessageQueue.obj"
	-@erase "$(INTDIR)\MessageQueue.sbr"
	-@erase "$(INTDIR)\Str.obj"
	-@erase "$(INTDIR)\Str.sbr"
	-@erase "$(INTDIR)\test_app.obj"
	-@erase "$(INTDIR)\test_app.sbr"
	-@erase "$(INTDIR)\TesterPipe.obj"
	-@erase "$(INTDIR)\TesterPipe.sbr"
	-@erase "$(INTDIR)\tests.obj"
	-@erase "$(INTDIR)\tests.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\Window.obj"
	-@erase "$(INTDIR)\Window.sbr"
	-@erase "$(OUTDIR)\tests-g.exe"
	-@erase "..\..\tmp\Debug\tests.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W4 /GR /Z7 /Od /I "..\..\include" /I\
 "..\..\..\externals\MicrosoftSDK\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "UNICODE" /D _WIN32_WINNT=0x501 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=..\..\tmp\Debug\tests/
CPP_SBRS=..\..\tmp\Debug\tests/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"..\..\tmp\Debug\tests.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Image.sbr" \
	"$(INTDIR)\manifest.sbr" \
	"$(INTDIR)\MessageQueue.sbr" \
	"$(INTDIR)\Str.sbr" \
	"$(INTDIR)\test_app.sbr" \
	"$(INTDIR)\TesterPipe.sbr" \
	"$(INTDIR)\tests.sbr" \
	"$(INTDIR)\Window.sbr"

"..\..\tmp\Debug\tests.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=jpeg-g.lib png-g.lib zlib-g.lib kernel32.lib user32.lib gdi32.lib\
 advapi32.lib comctl32.lib shell32.lib MSImg32.Lib rpcrt4.lib /nologo\
 /subsystem:console /pdb:none /debug /machine:I386 /out:"$(OUTDIR)\tests-g.exe"\
 /libpath:"..\..\lib" /libpath:"..\..\..\externals\MicrosoftSDK\lib" 
LINK32_OBJS= \
	"$(INTDIR)\Image.obj" \
	"$(INTDIR)\manifest.obj" \
	"$(INTDIR)\MessageQueue.obj" \
	"$(INTDIR)\Str.obj" \
	"$(INTDIR)\test_app.obj" \
	"$(INTDIR)\TesterPipe.obj" \
	"$(INTDIR)\tests.obj" \
	"$(INTDIR)\Window.obj"

"$(OUTDIR)\tests-g.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "tests - Win32 Release" || "$(CFG)" == "tests - Win32 Debug"
SOURCE=..\..\include\Image.cpp
DEP_CPP_IMAGE=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\..\externals\MicrosoftSDK\include\sys\types.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\jconfig.h"\
	"..\..\include\jerror.h"\
	"..\..\include\jmorecfg.h"\
	"..\..\include\jpeglib.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\png.h"\
	"..\..\include\pngconf.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	"..\..\include\zconf.h"\
	"..\..\include\zlib.h"\
	
NODEP_CPP_IMAGE=\
	"..\..\include\jpegint.h"\
	"..\..\include\pngusr.h"\
	

"$(INTDIR)\Image.obj"	"$(INTDIR)\Image.sbr" : $(SOURCE) $(DEP_CPP_IMAGE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\include\manifest.cpp
DEP_CPP_MANIF=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\manifest.obj"	"$(INTDIR)\manifest.sbr" : $(SOURCE) $(DEP_CPP_MANIF)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\include\MessageQueue.cpp
DEP_CPP_MESSA=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\MessageQueue.obj"	"$(INTDIR)\MessageQueue.sbr" : $(SOURCE)\
 $(DEP_CPP_MESSA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\include\Str.cpp
DEP_CPP_STR_C=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\Str.obj"	"$(INTDIR)\Str.sbr" : $(SOURCE) $(DEP_CPP_STR_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\include\TesterPipe.cpp
DEP_CPP_TESTE=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\TesterPipe.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\TesterPipe.obj"	"$(INTDIR)\TesterPipe.sbr" : $(SOURCE)\
 $(DEP_CPP_TESTE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\include\Window.cpp
DEP_CPP_WINDO=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\Window.obj"	"$(INTDIR)\Window.sbr" : $(SOURCE) $(DEP_CPP_WINDO)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!IF  "$(CFG)" == "tests - Win32 Release"

"BookSmart - Win32 Release" : 
   cd "\blurb\code\projects\BookSmart"
   $(MAKE) /$(MAKEFLAGS) /F .\BookSmart.mak CFG="BookSmart - Win32 Release" 
   cd "..\tests"

"BookSmart - Win32 ReleaseCLEAN" : 
   cd "\blurb\code\projects\BookSmart"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\BookSmart.mak\
 CFG="BookSmart - Win32 Release" RECURSE=1 
   cd "..\tests"

!ELSEIF  "$(CFG)" == "tests - Win32 Debug"

"BookSmart - Win32 Debug" : 
   cd "\blurb\code\projects\BookSmart"
   $(MAKE) /$(MAKEFLAGS) /F .\BookSmart.mak CFG="BookSmart - Win32 Debug" 
   cd "..\tests"

"BookSmart - Win32 DebugCLEAN" : 
   cd "\blurb\code\projects\BookSmart"
   $(MAKE) /$(MAKEFLAGS) CLEAN /F .\BookSmart.mak CFG="BookSmart - Win32 Debug"\
 RECURSE=1 
   cd "..\tests"

!ENDIF 

SOURCE=..\..\src\tests\test_app.cpp
DEP_CPP_TEST_=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\TesterPipe.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\test_app.obj"	"$(INTDIR)\test_app.sbr" : $(SOURCE) $(DEP_CPP_TEST_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\tests\tests.cpp
DEP_CPP_TESTS=\
	"..\..\..\externals\microsoftsdk\include\dbghelp.h"\
	"..\..\..\externals\microsoftsdk\include\psapi.h"\
	"..\..\..\externals\microsoftsdk\include\strsafe.h"\
	"..\..\include\Array.h"\
	"..\..\include\ByteArray.h"\
	"..\..\include\ByteArrayInputStream.h"\
	"..\..\include\Color.h"\
	"..\..\include\Comparable.h"\
	"..\..\include\DnD.h"\
	"..\..\include\Double.h"\
	"..\..\include\File.h"\
	"..\..\include\FileInputStream.h"\
	"..\..\include\FileOutputStream.h"\
	"..\..\include\HourglassCursor.h"\
	"..\..\include\Image.h"\
	"..\..\include\InputStream.h"\
	"..\..\include\List.h"\
	"..\..\include\Long.h"\
	"..\..\include\manifest.h"\
	"..\..\include\Map.h"\
	"..\..\include\MessageQueue.h"\
	"..\..\include\Object.h"\
	"..\..\include\OutputStream.h"\
	"..\..\include\Point.h"\
	"..\..\include\Rect.h"\
	"..\..\include\Rectangle2D.h"\
	"..\..\include\Set.h"\
	"..\..\include\Str.h"\
	"..\..\include\StrBuffer.h"\
	"..\..\include\Window.h"\
	"..\..\include\XMLDecoder.h"\
	"..\..\include\XMLEncoder.h"\
	

"$(INTDIR)\tests.obj"	"$(INTDIR)\tests.sbr" : $(SOURCE) $(DEP_CPP_TESTS)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

