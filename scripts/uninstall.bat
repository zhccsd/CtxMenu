@echo off
echo Installing CtxMenu...

regsvr32 /u /s %~dp0\CtxMenu.dll

if %ERRORLEVEL% equ 0 (
    echo CtxMenu uninstalled successfully!
) else (
    echo Failed to uninstall CtxMenu! %ERRORLEVEL%
)

pause