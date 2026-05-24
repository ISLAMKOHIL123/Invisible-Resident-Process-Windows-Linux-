@echo off
echo ========================================
echo Chargement du driver ProcessHider
echo ========================================
echo.

REM Arreter et supprimer si existe deja
sc stop ProcessHider >nul 2>&1
sc delete ProcessHider >nul 2>&1

REM Creer le service
echo Creation du service...
sc create ProcessHider type= kernel binPath= "C:\Drivers\ProcessHider.sys"

if errorlevel 1 (
    echo ERREUR lors de la creation!
    pause
    exit /b 1
)

REM Demarrer le driver
echo Demarrage du driver...
sc start ProcessHider

if errorlevel 1 (
    echo ERREUR lors du demarrage!
    echo Verifiez les evenements systeme.
    pause
    exit /b 1
)

echo.
echo ========================================
echo SUCCES! Driver charge!
echo notepad.exe est maintenant cache
echo ========================================
echo.
pause