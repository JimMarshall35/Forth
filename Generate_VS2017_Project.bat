@echo off
call Clean.bat
for %%I in (.) do set CurrDirName=%%~nxI
REM cmake -G "Visual Studio 15 2017" -S "vendor\googletest" -B "vendor\googletest\build"
call premake\premake5.exe vs2017 %CurrDirName%
rmdir /s /q x64
pause