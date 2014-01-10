@echo off

REM Build a 32 or 64 bit executable using MSVC

..\..\Source\EmbedDIBits.py .\octocat.png

cl /c /W4 /O1 /Os /GS- example.cpp

link /NODEFAULTLIB /ENTRY:EntryPoint  ^
     /SUBSYSTEM:windows               ^
     example.obj gdi32.lib kernel32.lib msimg32.lib user32.lib winmm.lib

del *.obj

