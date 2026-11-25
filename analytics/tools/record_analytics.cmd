@echo off
py -3 -B "%~dp0record_recent.py" capture --interactive %*
set "result=%errorlevel%"
pause
exit /b %result%
