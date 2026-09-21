@echo off
if exist "%~dp0Dist\Windows\LunarRescue.exe" (
    start "Lunar Rescue" "%~dp0Dist\Windows\LunarRescue.exe" -windowed -ResX=1280 -ResY=720 -dx11 -ExecCmds="r.Streaming.PoolSize 384,r.ShadowQuality 2"
    exit /b
)
start "Lunar Rescue" "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0LunarRescue.uproject" /Game/Lunar/Maps/MoonBase -game -windowed -ResX=1280 -ResY=720 -dx11 -ExecCmds="r.Streaming.PoolSize 384,r.ShadowQuality 2"
