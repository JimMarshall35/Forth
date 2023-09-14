@echo off
call Clean.bat
for %%I in (.) do set CurrDirName=%%~nxI
cmake -G "Visual Studio 17 2022" -S "vendor\glfw" -B "vendor\glfw\build"
call premake\premake5.exe vs2022 %CurrDirName%
rmdir /s /q x64
pause