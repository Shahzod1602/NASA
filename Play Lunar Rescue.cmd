@echo off
cd /d "%~dp0"
start "Lunar Rescue" "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0LunarRescue.uproject" /Game/Lunar/Maps/MoonBase -game -windowed -ResX=1280 -ResY=720 -dx11 -log
