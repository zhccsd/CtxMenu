@echo off
echo Installing CtxMenu...

regsvr32 /s %~dp0\CtxMenu.dll

if %ERRORLEVEL% equ 0 (
    echo CtxMenu installed successfully!
) else (
    echo Failed to install CtxMenu! %ERRORLEVEL%
)

pause