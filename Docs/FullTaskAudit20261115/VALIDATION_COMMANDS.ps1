$ErrorActionPreference='Stop'
Set-Location -LiteralPath D:/LunarRescue
$folder='D:/LunarRescue/Reports/FullTaskAudit20261115'
if(Test-Path -LiteralPath "$folder/completed-utc.txt"){throw 'Choose a new report folder; do not overwrite dated verification evidence'}
New-Item -ItemType Directory -Force -Path $folder | Out-Null
[DateTime]::UtcNow.ToString('o') | Set-Content -LiteralPath "$folder/started-utc.txt"
$env:DOTNET_GCHeapHardLimit='40000000'
& D:/UnrealEngine/UE_5.8/Engine/Build/BatchFiles/Build.bat LunarRescueEditor Win64 Development -Project=D:/LunarRescue/LunarRescue.uproject -WaitMutex -MaxParallelActions=1 -NoUBA -NoPCH -NoSharedPCH *> "$folder/editor-build.log"
if($LASTEXITCODE -ne 0){throw 'Editor build failed'}
& D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe D:/LunarRescue/LunarRescue.uproject -run=pythonscript -script=D:/LunarRescue/Tools/import_startup_feedback.py -unattended -nullrhi -nosound *> "$folder/import.log"
if($LASTEXITCODE -ne 0){throw 'UI artwork import failed'}
& ./Tools/verify_field_operations.ps1 -Review -Mission selene *> "$folder/editor-field-review.log"
Write-Output 'Editor reference layout and actual hatch checks passed'
$env:DOTNET_GCHeapHardLimit='40000000'
& D:/UnrealEngine/UE_5.8/Engine/Build/BatchFiles/Build.bat LunarRescue Win64 Development -Project=D:/LunarRescue/LunarRescue.uproject -WaitMutex -MaxParallelActions=1 -NoUBA -NoPCH -NoSharedPCH -NoDebugInfo -NoLinkerDebugInfo *> "$folder/game-build.log"
if($LASTEXITCODE -ne 0){throw 'Game build failed'}
& D:/UnrealEngine/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat BuildCookRun -project=D:/LunarRescue/LunarRescue.uproject -noP4 -platform=Win64 -clientconfig=Development -skipbuild -nocompileeditor -cook -map=/Game/Rescue/Maps/MissionSelect+/Game/Lunar/Maps/Apollo12+/Game/Lunar/Maps/MoonBase+/Game/Mars/Maps/MarsBase -unattended -utf8output '-AdditionalCookerOptions=-CookProcessCount=1' *> "$folder/cook.log"
if($LASTEXITCODE -ne 0){throw 'Cook failed'}
& ./Tools/package_apollo12.ps1 -StageOnly *> "$folder/stage.log"
Write-Output 'Authorized Windows demo built, cooked and staged'
& ./Tools/verify_field_operations.ps1 -Packaged -Mission all *> "$folder/field.log"
& ./Tools/verify_gameplay_tasks.ps1 -Packaged -Mission all *> "$folder/gameplay.log"
& ./Tools/verify_rescue_feedback.ps1 -Packaged *> "$folder/rescue.log"
& ./Tools/verify_task_fixes.ps1 -Packaged -Suite stride *> "$folder/stride.log"
& ./Tools/verify_mission_selection.ps1 -Packaged *> "$folder/selection.log"
& ./Tools/verify_nasa_tasks.ps1 -Packaged -Intro -TeamMediaDemo *> "$folder/startup.log"
Write-Output 'Packaged core, three-destination and media-presence checks passed'
& ./Tools/verify_field_operations.ps1 -Packaged -Review -Mission apollo *> "$folder/apollo-render.log"
& ./Tools/verify_field_operations.ps1 -Packaged -Review -Mission selene *> "$folder/selene-render.log"
& ./Tools/verify_field_operations.ps1 -Packaged -Review -Mission mars *> "$folder/mars-render.log"
& ./Tools/verify_nasa_tasks.ps1 -Packaged -Intro -Review -TeamMediaDemo *> "$folder/startup-render.log"
& ./Tools/verify_loading.ps1 -Packaged -Graphics -Intro *> "$folder/intro-loading.log"
& ./Tools/profile_apollo12.ps1 -Label full-task-demo-20261115 -Frames 1200 -Quality 1 -FieldOperationsProfile -FpsCap 60 -GpuTelemetry *> "$folder/performance.log"
[DateTime]::UtcNow.ToString('o') | Set-Content -LiteralPath "$folder/completed-utc.txt"
Write-Output 'ALL AUTHORIZED DEMO CHECKS PASSED'
