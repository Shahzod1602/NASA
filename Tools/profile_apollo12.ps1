param(
 [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='current',
 [ValidateRange(1200,12000)][int]$Frames=4200,
 [ValidateRange(-1,2)][int]$Quality=-1,
 [switch]$FastStride,
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
$captureArgs=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-ApolloWalkTest','-ApolloSaveProfile','-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height",'-nosound',"-ExecCmds=`"t.MaxFPS $FpsCap,r.VSync 0`"","-csvCaptureFrames=$Frames",'-csvGpuStats','-ExitAfterCsvProfiling',"-abslog=`"$log`"")
if($Quality -ge 0){$captureArgs+="-ApolloQuality=$Quality"}
if($FastStride){$captureArgs+='-ApolloProfileFastStride'}
$configPath=Join-Path $gameRoot 'Saved\Config\Windows\GameUserSettings.ini'
$configExisted=Test-Path -LiteralPath $configPath
$configBytes=if($configExisted){[System.IO.File]::ReadAllBytes($configPath)}else{$null}
$profilePath=Join-Path $gameRoot 'Saved\AutomatedTests\ApolloProfile'
$profileBackup=Join-Path $report 'CheckpointBefore'
New-Item -ItemType Directory -Force -Path $profileBackup | Out-Null
if(Test-Path -LiteralPath $profilePath){Get-ChildItem -LiteralPath $profilePath -File | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination $profileBackup -Force}}
$started=Get-Date
try {
$capture=Start-Process -FilePath $exe -ArgumentList $captureArgs -WindowStyle Hidden -PassThru
$peakWorkingSet=0L
$deadline=$started.AddMinutes(10)
while(-not $capture.WaitForExit(1000)){
 if((Get-Date) -gt $deadline){Stop-Process -Id $capture.Id;throw 'Graphics capture timed out'}
 $capture.Refresh();$peakWorkingSet=[Math]::Max($peakWorkingSet,$capture.WorkingSet64)
}
} finally {
 if($configExisted){New-Item -ItemType Directory -Force -Path (Split-Path $configPath) | Out-Null;[System.IO.File]::WriteAllBytes($configPath,$configBytes)}
 elseif(Test-Path -LiteralPath $configPath){Remove-Item -LiteralPath $configPath}
 if(Test-Path -LiteralPath $profilePath){Get-ChildItem -LiteralPath $profileBackup -File | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination $profilePath -Force}}
}
# Unreal can report a nonzero shutdown status after a successful CSV capture.
# Require a fresh complete capture and a clean log instead of relying on exit code.
$csv=Get-ChildItem -LiteralPath (Join-Path $gameRoot 'Saved\Profiling\CSV') -Filter '*.csv' | Where-Object {$_.LastWriteTime -ge $started} | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$logText=Get-Content -LiteralPath $log -Raw
if(-not $csv -or $logText -notmatch 'Capture Ended' -or $logText -match 'Fatal error:|Unhandled Exception:'){throw "No clean graphics capture; see $log"}
if($logText -notmatch ('systemresolution\.resx="'+$Width+'"') -or $logText -notmatch ('systemresolution\.resy="'+$Height+'"')){throw "Capture did not use the requested ${Width}x${Height} resolution; see $log"}
$savedCsv=Join-Path $report 'capture.csv'
Copy-Item -LiteralPath $csv.FullName -Destination $savedCsv
& python (Join-Path $PSScriptRoot 'summarize_profile.py') $savedCsv --expected-frames $Frames --fps-cap $FpsCap --output (Join-Path $report 'summary.json')
if($LASTEXITCODE -ne 0){throw 'CSV capture validation failed'}
[ordered]@{
 utc=(Get-Date).ToUniversalTime().ToString('o')
 cpu=(Get-CimInstance Win32_Processor | Select-Object -ExpandProperty Name)
 adapters=@(Get-CimInstance Win32_VideoController | Select-Object -ExpandProperty Name)
 ram_bytes=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
 process_peak_sampled_working_set_bytes=$peakWorkingSet
 renderer="DX11 / rendered offscreen / ${Width}x${Height} / configured scalability"
 fps_cap=$FpsCap
 quality=if($Quality -ge 0){$Quality}else{'existing configuration'}
 fast_stride=[bool]$FastStride
 command_arguments=$captureArgs
 process_exit_code=$capture.ExitCode
} | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $report 'environment.json') -Encoding UTF8
Write-Output "Graphics capture validated: $report"
