@echo off
call Clean.bat
for %%I in (.) do set CurrDirName=%%~nxI
REM cmake -G "Visual Studio 16 2019" -S "vendor\googletest" -B "vendor\googletest\build"
call premake\premake5.exe vs2019 %CurrDirName%
rmdir /s /q x64
pause