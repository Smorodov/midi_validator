@echo off
echo Building MIDI Validator...
echo.

if not exist build mkdir build
cd build

cmake .. -G "MinGW Makefiles"
if errorlevel 1 (
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

cd ..
echo.
echo Build successful!
echo Executable: build\midi_validator.exe
echo.
pause