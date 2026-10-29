param(
 [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='current',
 [ValidateRange(1200,12000)][int]$Frames=4200,
 [ValidateRange(-1,2)][int]$Quality=-1,
 [switch]$FastStride,
 [switch]$HideSkeletalMeshes,
 [switch]$Onscreen,
 [switch]$GpuTelemetry,
 [ValidatePattern('^[A-Za-z0-9_., =-]*$')][string]$DiagnosticCommands='',
 [ValidateRange(960,3840)][int]$Width=1280,
 [ValidateRange(540,2160)][int]$Height=720,
 [ValidateRange(0,240)][int]$FpsCap=60,
 [ValidateSet('en')][string]$Language='en'
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$gameRoot=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'
$exe=Join-Path $gameRoot 'Binaries\Win64\LunarRescue.exe'
$report=Join-Path $root "Reports\Performance\$Label"
New-Item -ItemType Directory -Path $report -Force | Out-Null
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
$log=Join-Path $report 'capture.log'
$busy=@(Get-Process -Name LunarRescue,UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker -ErrorAction SilentlyContinue)
if($busy.Count){throw 'Another game/editor/shader worker is running. Stop that work before a controlled profile.'}
$profileCommands="t.MaxFPS $FpsCap,r.VSync 0"
if($DiagnosticCommands){$profileCommands+=","+$DiagnosticCommands}
if($HideSkeletalMeshes){$profileCommands+=",show SkeletalMeshes"}
$captureArgs=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-ApolloWalkTest','-ApolloSaveProfile','-dx11','-ForceRes',"-ResX=$Width","-ResY=$Height",'-nosound',"-ExecCmds=`"$profileCommands`"","-csvCaptureFrames=$Frames",'-csvGpuStats','-ExitAfterCsvProfiling',"-abslog=`"$log`"")
if($Onscreen){$captureArgs+=@('-windowed')}else{$captureArgs+='-RenderOffscreen'}
if($Quality -ge 0){$captureArgs+="-ApolloQuality=$Quality"}
if($FastStride){$captureArgs+='-ApolloProfileFastStride'}
$configPath=Join-Path $gameRoot 'Saved\Config\Windows\GameUserSettings.ini'
$configExisted=Test-Path -LiteralPath $configPath
$configBytes=if($configExisted){[System.IO.File]::ReadAllBytes($configPath)}else{$null}
$profilePath=Join-Path $gameRoot 'Saved\AutomatedTests\ApolloProfile'
$profileBackup=Join-Path $report 'CheckpointBefore'
New-Item -ItemType Directory -Force -Path $profileBackup | Out-Null
if(Test-Path -LiteralPath $profilePath){Get-ChildItem -LiteralPath $profilePath -File | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination $profileBackup -Force}}
$gpuCommand=Get-Command nvidia-smi -ErrorAction SilentlyContinue
$gpuSamples=@();$otherGameSeen=$false;$sampleTick=0
$started=Get-Date
try {
$windowStyle=if($Onscreen){'Normal'}else{'Hidden'}
$capture=Start-Process -FilePath $exe -ArgumentList $captureArgs -WindowStyle $windowStyle -PassThru
$peakWorkingSet=0L
$deadline=$started.AddMinutes(10)
while(-not $capture.WaitForExit(1000)){
 if((Get-Date) -gt $deadline){Stop-Process -Id $capture.Id;$capture.WaitForExit();throw 'Graphics capture timed out'}
 $capture.Refresh();$peakWorkingSet=[Math]::Max($peakWorkingSet,$capture.WorkingSet64)
 if(@(Get-Process -Name LunarRescue,UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker -ErrorAction SilentlyContinue | Where-Object {$_.Id -ne $capture.Id}).Count){$otherGameSeen=$true}
 $sampleTick++
 if($GpuTelemetry -and $gpuCommand -and $sampleTick%5 -eq 0){
  $reading=& $gpuCommand.Source --query-gpu=timestamp,name,temperature.gpu,utilization.gpu,clocks.gr,clocks.mem,power.draw --format=csv,noheader,nounits 2>$null
  if($LASTEXITCODE -eq 0 -and $reading){$gpuSamples+=@($reading)}
 }
}
} finally {
 if($configExisted){New-Item -ItemType Directory -Force -Path (Split-Path $configPath) | Out-Null;[System.IO.File]::WriteAllBytes($configPath,$configBytes)}
 elseif(Test-Path -LiteralPath $configPath){Remove-Item -LiteralPath $configPath}
 if(Test-Path -LiteralPath $profilePath){Get-ChildItem -LiteralPath $profileBackup -File | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination $profilePath -Force}}
}
# Unreal can report a nonzero shutdown status after a successful CSV capture.
# Require a fresh complete capture and a clean log instead of relying on exit code.
@('timestamp,name,temperature_c,gpu_percent,graphics_mhz,memory_mhz,power_w')+$gpuSamples | Set-Content (Join-Path $report 'gpu-samples.csv') -Encoding UTF8
if($otherGameSeen){throw 'Capture overlapped another game/editor/shader process; do not use as a baseline.'}
$csv=Get-ChildItem -LiteralPath (Join-Path $gameRoot 'Saved\Profiling\CSV') -Filter '*.csv' | Where-Object {$_.LastWriteTime -ge $started} | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$logText=Get-Content -LiteralPath $log -Raw
if(-not $csv -or $logText -notmatch 'Capture Ended' -or $logText -match 'Fatal error:|Unhandled Exception:'){throw "No clean graphics capture; see $log"}
if($logText -notmatch ('systemresolution\.resx="'+$Width+'"') -or $logText -notmatch ('systemresolution\.resy="'+$Height+'"')){throw "Capture did not use the requested ${Width}x${Height} resolution; see $log"}
$savedCsv=Join-Path $report 'capture.csv'
Copy-Item -LiteralPath $csv.FullName -Destination $savedCsv
$presentationArgs=@();if($Onscreen){$presentationArgs+='--onscreen'}
& python (Join-Path $PSScriptRoot 'summarize_profile.py') $savedCsv --expected-frames $Frames --fps-cap $FpsCap --output (Join-Path $report 'summary.json') @presentationArgs
if($LASTEXITCODE -ne 0){throw 'CSV capture validation failed'}
[ordered]@{
 utc=(Get-Date).ToUniversalTime().ToString('o')
 cpu=(Get-CimInstance Win32_Processor | Select-Object -ExpandProperty Name)
 adapters=@(Get-CimInstance Win32_VideoController | Select-Object -ExpandProperty Name)
 ram_bytes=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
 process_peak_sampled_working_set_bytes=$peakWorkingSet
 renderer="DX11 / $(if($Onscreen){'on-screen window'}else{'rendered offscreen'}) / ${Width}x${Height} / configured scalability"
 onscreen=[bool]$Onscreen
 gpu_samples=$gpuSamples.Count
 gpu_telemetry_requested=[bool]$GpuTelemetry
 diagnostic_commands=$DiagnosticCommands
 fps_cap=$FpsCap
 quality=if($Quality -ge 0){$Quality}else{'existing configuration'}
 fast_stride=[bool]$FastStride
 diagnostic_hide_skeletal_meshes=[bool]$HideSkeletalMeshes
 command_arguments=$captureArgs
 process_exit_code=$capture.ExitCode
} | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $report 'environment.json') -Encoding UTF8
Write-Output "Graphics capture validated: $report"
