@echo off
set "folder=.\Logs"
set "file=.gitkeep"
del /q "%folder%\*"
echo.  > "%folder%\%file%"
echo Cleared Logs!
