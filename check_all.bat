@echo off
set VALIDATOR=build\midi_validator.exe
set TEST_DIR=tests\build\tests\generated
set LOG_FILE=check_all.log

echo Checking all MIDI files > %LOG_FILE%
echo Date: %date% %time% >> %LOG_FILE%
echo. >> %LOG_FILE%

echo ========================================
echo Checking all MIDI files
echo ========================================
echo.

for %%f in (%TEST_DIR%\*.mid) do (
    echo [%%~nxf]
    echo ======================================== >> %LOG_FILE%
    echo File: %%~nxf >> %LOG_FILE%
    echo ======================================== >> %LOG_FILE%
    
    %VALIDATOR% "%%f" >> %LOG_FILE% 2>&1
    echo Exit code: %errorlevel% >> %LOG_FILE%
    echo. >> %LOG_FILE%
    
    echo   Exit code: %errorlevel%
    echo.
)

echo Log saved to: %LOG_FILE%
pause