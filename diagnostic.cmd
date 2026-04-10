@echo off
setlocal enabledelayedexpansion

set TEST_DIR=tests\build\tests\generated
set LOG_FILE=diagnostic_log.txt

echo MIDI Validator Diagnostic Tool
echo ==============================
echo.

echo MIDI Validator Diagnostic Tool > "%LOG_FILE%"
echo ============================== >> "%LOG_FILE%"
echo Date: %date% %time% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo Checking generated files:
echo.

echo Checking generated files: >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

if exist "%TEST_DIR%\valid_format0.mid" (
    echo [OK] valid_format0.mid exists
    echo [OK] valid_format0.mid exists >> "%LOG_FILE%"
    for %%A in ("%TEST_DIR%\valid_format0.mid") do (
        echo   Size: %%~zA bytes
        echo   Size: %%~zA bytes >> "%LOG_FILE%"
    )
) else (
    echo [MISSING] valid_format0.mid
    echo [MISSING] valid_format0.mid >> "%LOG_FILE%"
)

if exist "%TEST_DIR%\valid_format1.mid" (
    echo [OK] valid_format1.mid exists
    echo [OK] valid_format1.mid exists >> "%LOG_FILE%"
    for %%A in ("%TEST_DIR%\valid_format1.mid") do (
        echo   Size: %%~zA bytes
        echo   Size: %%~zA bytes >> "%LOG_FILE%"
    )
) else (
    echo [MISSING] valid_format1.mid
    echo [MISSING] valid_format1.mid >> "%LOG_FILE%"
)

if exist "%TEST_DIR%\valid_tempo.mid" (
    echo [OK] valid_tempo.mid exists
    echo [OK] valid_tempo.mid exists >> "%LOG_FILE%"
    for %%A in ("%TEST_DIR%\valid_tempo.mid") do (
        echo   Size: %%~zA bytes
        echo   Size: %%~zA bytes >> "%LOG_FILE%"
    )
) else (
    echo [MISSING] valid_tempo.mid
    echo [MISSING] valid_tempo.mid >> "%LOG_FILE%"
)

if exist "%TEST_DIR%\valid_multitrack.mid" (
    echo [OK] valid_multitrack.mid exists
    echo [OK] valid_multitrack.mid exists >> "%LOG_FILE%"
    for %%A in ("%TEST_DIR%\valid_multitrack.mid") do (
        echo   Size: %%~zA bytes
        echo   Size: %%~zA bytes >> "%LOG_FILE%"
    )
) else (
    echo [MISSING] valid_multitrack.mid
    echo [MISSING] valid_multitrack.mid >> "%LOG_FILE%"
)

echo.
echo ========================================
echo Running validator tests...
echo ========================================
echo. >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
echo Running validator tests... >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"

echo.
echo [TEST 1] valid_format0.mid
echo [TEST 1] valid_format0.mid >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\valid_format0.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% equ 0 (
    echo   RESULT: PASS (expected 0)
    echo   RESULT: PASS (expected 0) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%)
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo [TEST 2] valid_format1.mid
echo [TEST 2] valid_format1.mid >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\valid_format1.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% equ 0 (
    echo   RESULT: PASS (expected 0)
    echo   RESULT: PASS (expected 0) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%)
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo [TEST 3] valid_tempo.mid
echo [TEST 3] valid_tempo.mid >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\valid_tempo.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% equ 0 (
    echo   RESULT: PASS (expected 0)
    echo   RESULT: PASS (expected 0) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%)
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo [TEST 4] valid_multitrack.mid
echo [TEST 4] valid_multitrack.mid >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\valid_multitrack.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% equ 0 (
    echo   RESULT: PASS (expected 0)
    echo   RESULT: PASS (expected 0) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%)
    echo   RESULT: FAIL (expected 0, got %EXIT_CODE%) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo [TEST 5] invalid_no_eot.mid (should FAIL)
echo [TEST 5] invalid_no_eot.mid (should FAIL) >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\invalid_no_eot.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% neq 0 (
    echo   RESULT: PASS (expected non-zero)
    echo   RESULT: PASS (expected non-zero) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected non-zero, got 0)
    echo   RESULT: FAIL (expected non-zero, got 0) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo [TEST 6] invalid_note_range.mid (should FAIL)
echo [TEST 6] invalid_note_range.mid (should FAIL) >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\invalid_note_range.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% neq 0 (
    echo   RESULT: PASS (expected non-zero)
    echo   RESULT: PASS (expected non-zero) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected non-zero, got 0)
    echo   RESULT: FAIL (expected non-zero, got 0) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo [TEST 7] invalid_running_status.mid (should FAIL)
echo [TEST 7] invalid_running_status.mid (should FAIL) >> "%LOG_FILE%"
build\midi_validator.exe "%TEST_DIR%\invalid_running_status.mid" > temp_output.txt 2>&1
set EXIT_CODE=%ERRORLEVEL%
echo Exit code: %EXIT_CODE%
echo Exit code: %EXIT_CODE% >> "%LOG_FILE%"
if %EXIT_CODE% neq 0 (
    echo   RESULT: PASS (expected non-zero)
    echo   RESULT: PASS (expected non-zero) >> "%LOG_FILE%"
) else (
    echo   RESULT: FAIL (expected non-zero, got 0)
    echo   RESULT: FAIL (expected non-zero, got 0) >> "%LOG_FILE%"
)
echo Output:
type temp_output.txt
echo. >> "%LOG_FILE%"
echo Output: >> "%LOG_FILE%"
type temp_output.txt >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo.
echo ========================================
echo Diagnostic complete!
echo Log saved to: %LOG_FILE%
echo ========================================
echo.
echo ======================================== >> "%LOG_FILE%"
echo Diagnostic complete! >> "%LOG_FILE%"
echo Log saved to: %LOG_FILE% >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"

del temp_output.txt 2>nul

pause