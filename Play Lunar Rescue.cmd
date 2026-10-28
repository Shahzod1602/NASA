@echo off
cd /d "%~dp0"
if exist "%~dp0Dist\Apollo12\Windows\LunarRescue.exe" (
    start "Lunar Rescue - Apollo 12" "%~dp0Dist\Apollo12\Windows\LunarRescue.exe" -dx11
    exit /b
)
if exist "%~dp0Dist\MoonExpedition\Windows\LunarRescue.exe" (
    start "Lunar Rescue - The Lost Expedition" "%~dp0Dist\MoonExpedition\Windows\LunarRescue.exe" -dx11
    exit /b
)
if exist "%~dp0Dist\Windows\LunarRescue.exe" (
    start "Lunar Rescue" "%~dp0Dist\Windows\LunarRescue.exe" -dx11
    exit /b
)
start "Lunar Rescue" "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0LunarRescue.uproject" /Game/Rescue/Maps/MissionSelect -game -dx11 -log
