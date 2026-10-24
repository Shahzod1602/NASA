param(
 [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='current',
 [ValidateRange(1200,12000)][int]$Frames=4200,
 [ValidateSet('uz','en')][string]$Language='uz'
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$gameRoot=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'
$exe=Join-Path $gameRoot 'Binaries\Win64\LunarRescue.exe'
$report=Join-Path $root "Reports\Performance\$Label"
New-Item -ItemType Directory -Path $report -Force | Out-Null
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
$log=Join-Path $report 'capture.log'
$captureArgs=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-ApolloWalkTest','-ApolloSaveProfile',"-ApolloLanguage=$Language",'-dx11','-RenderOffscreen','-ForceRes','-ResX=1280','-ResY=720','-nosound','-ExecCmds="t.MaxFPS 60,r.VSync 0"',"-csvCaptureFrames=$Frames",'-csvGpuStats','-ExitAfterCsvProfiling',"-abslog=`"$log`"")
$started=Get-Date
$capture=Start-Process -FilePath $exe -ArgumentList $captureArgs -WindowStyle Hidden -PassThru
$peakWorkingSet=0L
$deadline=$started.AddMinutes(10)
while(-not $capture.WaitForExit(1000)){
 if((Get-Date) -gt $deadline){Stop-Process -Id $capture.Id;throw 'Graphics capture timed out'}
 $capture.Refresh();$peakWorkingSet=[Math]::Max($peakWorkingSet,$capture.WorkingSet64)
}
# Unreal can report a nonzero shutdown status after a successful CSV capture.
# Require a fresh complete capture and a clean log instead of relying on exit code.
$csv=Get-ChildItem -LiteralPath (Join-Path $gameRoot 'Saved\Profiling\CSV') -Filter '*.csv' | Where-Object {$_.LastWriteTime -ge $started} | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$logText=Get-Content -LiteralPath $log -Raw
if(-not $csv -or $logText -notmatch 'Capture Ended' -or $logText -match 'Fatal error:|Unhandled Exception:'){throw "No clean graphics capture; see $log"}
if($logText -notmatch 'systemresolution\.resx="1280"' -or $logText -notmatch 'systemresolution\.resy="720"'){throw "Capture did not use the requested 1280x720 resolution; see $log"}
$savedCsv=Join-Path $report 'capture.csv'
Copy-Item -LiteralPath $csv.FullName -Destination $savedCsv
& python (Join-Path $PSScriptRoot 'summarize_profile.py') $savedCsv --expected-frames $Frames --output (Join-Path $report 'summary.json')
if($LASTEXITCODE -ne 0){throw 'CSV capture validation failed'}
[ordered]@{
 utc=(Get-Date).ToUniversalTime().ToString('o')
 cpu=(Get-CimInstance Win32_Processor | Select-Object -ExpandProperty Name)
 adapters=@(Get-CimInstance Win32_VideoController | Select-Object -ExpandProperty Name)
 ram_bytes=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
 process_peak_sampled_working_set_bytes=$peakWorkingSet
 renderer='DX11 / rendered offscreen / 1280x720 / current configured scalability'
 fps_cap=60
 command_arguments=$captureArgs
 process_exit_code=$capture.ExitCode
} | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $report 'environment.json') -Encoding UTF8
Write-Output "Graphics capture validated: $report"
