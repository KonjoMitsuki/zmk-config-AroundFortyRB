@echo off
py -3 -B "%~dp0record_recent.py" build --publish %*
set "result=%errorlevel%"
pause
exit /b %result%
