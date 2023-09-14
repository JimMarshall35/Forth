@echo off
call Clean.bat
for %%I in (.) do set CurrDirName=%%~nxI
cmake -G "Visual Studio 15 2017" -S "vendor\glfw" -B "vendor\glfw\build"
call premake\premake5.exe vs2017 %CurrDirName%
rmdir /s /q x64
pause