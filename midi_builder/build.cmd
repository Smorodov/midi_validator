@echo off
echo Building  (Static)...
echo.

set LOGFILE=build_log.txt

echo > %LOGFILE%

if not exist build mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++" --log-level=VERBOSE >> ..\%LOGFILE% 2>&1
if errorlevel 1 (
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

cmake --build . --verbose -- -j4 >> ..\%LOGFILE% 2>&1
if errorlevel 1 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

cd ..
echo Build complete!
pause