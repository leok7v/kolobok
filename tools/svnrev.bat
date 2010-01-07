@echo off
rem current invokation directory is: ..\..\kolobok\projects\kolobok
rem uncomment next line to verify
rem cd
if not exist ..\..\tmp\kolobok mkdir ..\..\tmp\kolobok
SubWCRev.exe ..\.. %1 ..\..\tmp\kolobok\~svn.tmp  >nul

if not exist %2 goto :non_equal

..\..\tools\diff.exe -q ..\..\tmp\kolobok\~svn.tmp %2 >nul

if %errorlevel% equ 0 goto equal
    del %2
:non_equal
    rem echo NON EQUAL
    move ..\..\tmp\kolobok\~svn.tmp %2
    rem ..\..\tools\touch.exe ..\..\src\kolobok\resources\Application.rc
    goto :done
:equal
    rem echo EQUAL

:done
    if exist ..\..\tmp\kolobok\~svn.tmp del ..\..\tmp\kolobok\~svn.tmp
