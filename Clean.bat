@echo off
for %%I in (.) do set CurrDirName=%%~nxI
call premake\premake5.exe clean %CurrDirName%
rmdir /s /q x64