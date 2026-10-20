@echo off
set "BASE_DIR=%~dp0"
set "EXE_PATH=%BASE_DIR%bin\Password_Locker.exe"
if not exist "%EXE_PATH%" (
    set "EXE_PATH=%BASE_DIR%bin\EC_Lab_Vault.exe"
)

if not exist "%EXE_PATH%" (
    echo Executable not found at %EXE_PATH%!
    echo Running build.bat first...
    call "%BASE_DIR%build.bat"
    if %ERRORLEVEL% neq 0 (
        echo Build failed. Cannot launch application.
        pause
        exit /b 1
    )
    set "EXE_PATH=%BASE_DIR%bin\Password_Locker.exe"
)

echo Starting Password Locker - Offline Password Vault & 2FA Authenticator...
cd /d "%BASE_DIR%"
start "" "%EXE_PATH%"
exit /b 0
