param(
 [Parameter(Mandatory=$true)][ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label,
 [ValidateRange(1200,12000)][int]$Frames=1800,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720,
 [ValidateRange(0,240)][int]$FpsCap=60,
 [ValidateRange(0,2)][int]$Quality=1
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$gameRoot=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'
$exe=Join-Path $gameRoot 'Binaries\Win64\LunarRescue.exe'
$report=Join-Path $root "Reports\MissionSelection20261031\performance-$Label"
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing packaged executable: $exe"}
if(Test-Path -LiteralPath $report){throw "Capture label already exists; choose a fresh label: $report"}
function Get-ProfileBusyProcesses([int]$IgnoreId=0){
 $native=@(Get-Process -Name LunarRescue,UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker,cl,link -ErrorAction SilentlyContinue | Where-Object {$_.Id -ne $IgnoreId})
 $managed=@(Get-CimInstance Win32_Process -Filter "Name='dotnet.exe'" | Where-Object {$_.CommandLine -match 'UnrealBuildTool|AutomationTool|BuildCookRun'})
 return @($native)+@($managed)
}
if(@(Get-ProfileBusyProcesses).Count){throw 'Game/editor/shader/compiler/build work is running; finish it before a controlled menu capture.'}
New-Item -ItemType Directory -Path $report | Out-Null
$environment=[ordered]@{
 map='MissionSelect';scene='Static home menu; no mission selection or gameplay';cpu=(Get-CimInstance Win32_Processor).Name
 adapters=@((Get-CimInstance Win32_VideoController).Name);ram_bytes=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
 renderer='DX11 / rendered offscreen';resolution="${Width}x${Height}";quality=$Quality;fps_cap=$FpsCap;vsync=$false;audio=$false
 executable=$exe;executable_sha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
 requested_frames=$Frames;warmup_frames_excluded=300;gpu_telemetry_requested=$false
 memory_method='Maximum of one-second process working-set samples; not true peak, VRAM or dedicated GPU memory'
}
$log=Join-Path $report 'capture.log'
$arguments=@('/Game/Rescue/Maps/MissionSelect','-game','-unattended','-dx11','-RenderOffscreen','-nosound','-ForceRes',"-ResX=$Width","-ResY=$Height","-ApolloQuality=$Quality","-ExecCmds=`"t.MaxFPS $FpsCap,r.VSync 0`"","-csvCaptureFrames=$Frames",'-csvGpuStats','-ExitAfterCsvProfiling',"-abslog=`"$log`"")
$config=Join-Path $gameRoot 'Saved\Config\Windows\GameUserSettings.ini'
$hadConfig=Test-Path -LiteralPath $config -PathType Leaf
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$samples=@();$maximumSampledWorkingSet=0L;$overlapped=$false;$capture=$null
$started=Get-Date;$environment.started_utc=$started.ToUniversalTime().ToString('o');$environment.command_arguments=$arguments
try{
 if(@(Get-ProfileBusyProcesses).Count){throw 'A competing build/editor/game started during capture preparation.'}
 $capture=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
 while(-not $capture.WaitForExit(1000)){
  if((Get-Date) -gt $started.AddSeconds(180)){Stop-Process -Id $capture.Id;$capture.WaitForExit();throw 'Menu CSV capture exceeded 180 seconds; only its own process was stopped.'}
  $capture.Refresh();$maximumSampledWorkingSet=[Math]::Max($maximumSampledWorkingSet,$capture.WorkingSet64)
  $samples+=[ordered]@{elapsed_seconds=((Get-Date)-$started).TotalSeconds;working_set_bytes=$capture.WorkingSet64}
  if(@(Get-ProfileBusyProcesses -IgnoreId $capture.Id).Count){$overlapped=$true}
 }
}finally{
 if($hadConfig){New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null;[IO.File]::WriteAllBytes($config,$configBytes)}
 elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
 $environment.process_exit_code=if($capture -and $capture.HasExited){$capture.ExitCode}else{$null}
 $environment.maximum_sampled_working_set_bytes=$maximumSampledWorkingSet;$environment.competing_work_seen=$overlapped
 $environment | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $report 'environment.json') -Encoding UTF8
 ConvertTo-Json -InputObject @($samples) -Depth 3 | Set-Content -LiteralPath (Join-Path $report 'memory-samples.json') -Encoding UTF8
}
if($overlapped){throw "Capture overlapped competing game/editor/shader/compiler work; discard comparison: $report"}
# CSV auto-stop can return 777003. Record it unchanged; capture validation is not a functional-test exit claim.
if($capture.ExitCode -notin @(0,777003)){throw "Unexpected capture process exit $($capture.ExitCode); see $log"}
$csv=Get-ChildItem -LiteralPath (Join-Path $gameRoot 'Saved\Profiling\CSV') -Filter '*.csv' | Where-Object {$_.LastWriteTime -ge $started} | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$logText=Get-Content -LiteralPath $log -Raw
if(-not $csv -or $logText -notmatch 'Capture Ended' -or $logText -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material'){throw "No clean fresh completed CSV capture; see $log"}
if($logText -notmatch ('systemresolution\.resx="'+$Width+'"') -or $logText -notmatch ('systemresolution\.resy="'+$Height+'"')){throw "Requested resolution was not confirmed in $log"}
$destination=Join-Path $report 'capture.csv';Copy-Item -LiteralPath $csv.FullName -Destination $destination
& python (Join-Path $PSScriptRoot 'summarize_profile.py') $destination --warmup 300 --expected-frames $Frames --fps-cap $FpsCap --output (Join-Path $report 'summary.json') | Out-Null
if($LASTEXITCODE -ne 0){throw 'Menu CSV frame count or frame-time validation failed'}
$summary=Get-Content -LiteralPath (Join-Path $report 'summary.json') -Raw | ConvertFrom-Json
$summary.limitations="Rendered offscreen static home menu, $FpsCap FPS cap, VSync off; 300 warmup frames excluded. One controlled run, not whole-mission performance, a human playtest or animation smoothness validation. Working set is not VRAM or true peak."
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $report 'summary.json') -Encoding UTF8
Write-Output "Menu CSV capture validated ($($summary.captured_frames) frames; raw process exit $($capture.ExitCode)): $report"
