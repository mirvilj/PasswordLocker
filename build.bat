@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   EC_Lab Offline Password Vault - Build Script
echo ===================================================

set "VS_VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VS_VCVARS%" (
    echo Error: Could not locate Visual Studio 2022 vcvars64.bat at:
    echo "%VS_VCVARS%"
    exit /b 1
)

echo Initializing MSVC x64 build environment...
call "%VS_VCVARS%" >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error initializing MSVC environment.
    exit /b 1
)

set "BASE_DIR=%~dp0"
set "SRC_DIR=%BASE_DIR%src"
set "WX_DIR=%BASE_DIR%wxWidgets"
set "BIN_DIR=%BASE_DIR%bin"
set "DATA_DIR=%BASE_DIR%data"

if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%DATA_DIR%" mkdir "%DATA_DIR%"

echo Compiling Windows resources...
rc /nologo /fo"%BIN_DIR%\app.res" /i"%SRC_DIR%" /i"%WX_DIR%\include" "%SRC_DIR%\app.rc"
if %ERRORLEVEL% neq 0 (
    echo Warning: app.res compilation had warnings or errors. Continuing...
)

echo Compiling C++ source files...
cl /nologo /EHsc /MD /std:c++17 /O2 /utf-8 ^
   /DWXUSINGDLL /D__WXMSW__ /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS ^
   /I"%WX_DIR%\include" ^
   /I"%WX_DIR%\lib\vc14x_x64_dll\mswu" ^
   /I"%SRC_DIR%" ^
   /Fo"%BIN_DIR%\\" ^
   "%SRC_DIR%\TotpEngine.cpp" ^
   "%SRC_DIR%\VaultCrypto.cpp" ^
   "%SRC_DIR%\VaultItem.cpp" ^
   "%SRC_DIR%\VaultStorage.cpp" ^
   "%SRC_DIR%\ThemeManager.cpp" ^
   "%SRC_DIR%\ClipboardSanitizer.cpp" ^
   "%SRC_DIR%\PinDialog.cpp" ^
   "%SRC_DIR%\SettingsDialog.cpp" ^
   "%SRC_DIR%\BackupRestoreDialog.cpp" ^
   "%SRC_DIR%\PasswordAnalyzerDialog.cpp" ^
   "%SRC_DIR%\HelpDialog.cpp" ^
   "%SRC_DIR%\EntryDialog.cpp" ^
   "%SRC_DIR%\DetailDialog.cpp" ^
   "%SRC_DIR%\ImageViewerDialog.cpp" ^
   "%SRC_DIR%\TotpHubDialog.cpp" ^
   "%SRC_DIR%\ItemCardPanel.cpp" ^
   "%SRC_DIR%\MainWindow.cpp" ^
   "%SRC_DIR%\main.cpp" ^
   "%BIN_DIR%\app.res" ^
   /link ^
   /LIBPATH:"%WX_DIR%\lib\vc14x_x64_dll" ^
   wxmsw32u_core.lib wxbase32u.lib wxpng.lib wxjpeg.lib wxzlib.lib bcrypt.lib ^
   comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib user32.lib gdi32.lib advapi32.lib ^
   /SUBSYSTEM:WINDOWS ^
   /OUT:"%BIN_DIR%\Password_Locker.exe"

if %ERRORLEVEL% neq 0 (
    echo.
    echo Compilation failed with errors!
    exit /b %ERRORLEVEL%
)

echo.
echo Copying wxWidgets runtime DLLs to output directory...
copy /Y "%WX_DIR%\lib\vc14x_x64_dll\wxbase32u_vc14x_x64.dll" "%BIN_DIR%\" >nul
copy /Y "%WX_DIR%\lib\vc14x_x64_dll\wxmsw32u_core_vc14x_x64.dll" "%BIN_DIR%\" >nul
copy /Y "%BIN_DIR%\Password_Locker.exe" "%BIN_DIR%\EC_Lab_Vault.exe" >nul

echo.
echo ===================================================
echo   Build Successful! Executable ready at:
echo   %BIN_DIR%\Password_Locker.exe
echo ===================================================
exit /b 0
