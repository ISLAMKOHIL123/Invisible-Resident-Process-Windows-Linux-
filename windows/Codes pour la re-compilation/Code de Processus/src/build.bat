@echo off
REM Add MinGW or TDM-GCC to PATH if needed (edit the path below)
:: set PATH=C:\MinGW\bin;%PATH%

echo Compiling...
gcc main.c process.c -o program.exe -lpsapi -lkernel32

if %errorlevel% neq 0 (
    echo.
    echo ❌ Compilation failed!
    pause
    exit /b
)

echo.
echo ✅ Compilation successful!
echo Running program...
program.exe

pause
