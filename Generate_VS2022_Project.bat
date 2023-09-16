@echo off
call Clean.bat
for %%I in (.) do set CurrDirName=%%~nxI
REM cmake -G "Visual Studio 17 2022" -S "vendor\googletest" -B "vendor\googletest\build"
call premake\premake5.exe vs2022 %CurrDirName%
rmdir /s /q x64
pause