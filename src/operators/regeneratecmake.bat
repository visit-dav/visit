@echo off
set visitcmd="C:/A_VisIt/GIT2/build/exe/Release/visit.exe"

set back=%cd%
for /d %%i in (*) do (
  cd "%%i"
  @rem echo %%i.xml
  %visitcmd% -xml2cmake -clobber %%i.xml
  @rem echo current directory: 
  @rem cd
  cd %back%
)
