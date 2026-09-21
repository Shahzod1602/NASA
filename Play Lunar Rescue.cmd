@echo off
cd /d "%~dp0"
if exist "%~dp0Dist\Windows\LunarRescue.exe" (
    start "Lunar Rescue" "%~dp0Dist\Windows\LunarRescue.exe" -windowed -ResX=1280 -ResY=720 -dx11
    exit /b
)
start "Lunar Rescue" "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0LunarRescue.uproject" /Game/Rescue/Maps/MissionSelect -game -windowed -ResX=1280 -ResY=720 -dx11 -log
