@echo off
echo ========================================
echo Compilation du driver ProcessHider (x64)
echo ========================================
echo.

REM Configuration de l'environnement Visual Studio pour x64
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo ERREUR: Visual Studio introuvable!
    pause
    exit /b 1
)

REM Configuration WDK
set WDK_PATH=C:\Program Files (x86)\Windows Kits\10
set WDK_VERSION=10.0.26100.0
set INCLUDE=%WDK_PATH%\Include\%WDK_VERSION%\km;%WDK_PATH%\Include\%WDK_VERSION%\shared;%INCLUDE%
set LIB=%WDK_PATH%\Lib\%WDK_VERSION%\km\x64;%LIB%

echo Configuration terminee.
echo.
echo Verification de l'architecture...
cl.exe 2>&1 | findstr /C:"x64" >nul
if errorlevel 1 (
    echo ERREUR: Environnement x64 non configure!
    pause
    exit /b 1
)
echo Architecture x64 confirmee.
echo.

REM Compilation
echo Compilation de Driver.c...
cl.exe /c /nologo /W3 /O2 /D_AMD64_ /D_WIN64 /DWIN64 /kernel /Zp8 /Gy /Gm- /Zl /GR- /GS- /Gz /Fo"ProcessHider.obj" Driver.c

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERREUR de compilation!
    echo ========================================
    pause
    exit /b 1
)

REM Edition de liens
echo.
echo Edition de liens...
link.exe /NOLOGO /DRIVER /ENTRY:DriverEntry /SUBSYSTEM:NATIVE /MACHINE:X64 /OUT:ProcessHider.sys ntoskrnl.lib hal.lib ProcessHider.obj

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERREUR d'edition de liens!
    echo ========================================
    pause
    exit /b 1
)

echo.
if exist ProcessHider.sys (
    echo ========================================
    echo SUCCES! ProcessHider.sys a ete cree!
    dir ProcessHider.sys | find ".sys"
    echo ========================================
) else (
    echo ========================================
    echo ECHEC de la compilation!
    echo ========================================
)

echo.
pause