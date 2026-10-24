param([ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='v050-medium',[ValidateRange(0,2)][int]$Quality=1,[switch]$HistoricalRoute)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$gameRoot=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'
$report=Join-Path $root "Reports\Performance\$Label"
New-Item -ItemType Directory -Force -Path $report | Out-Null
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
$log=Join-Path $report 'capture.log'
$arguments=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-ApolloWalkTest','-ApolloSaveProfile','-ApolloFullProfile','-ApolloProfileFastStride','-dx11','-RenderOffscreen','-ForceRes','-ResX=1280','-ResY=720','-nosound',"-ApolloQuality=$Quality",'-ExecCmds="t.MaxFPS 60,r.VSync 0,csvprofile start"',"-abslog=`"$log`"")
if($HistoricalRoute){$arguments+='-ApolloHistoricalRoute'}
$started=Get-Date
$configPath=Join-Path $gameRoot 'Saved\Config\Windows\GameUserSettings.ini'
$configExisted=Test-Path -LiteralPath $configPath
$configBytes=if($configExisted){[System.IO.File]::ReadAllBytes($configPath)}else{$null}
$peak=0L;$gpuPeak=$null;$ticks=0;$gpuAvailable=$true;$samples=@()
try {
$p=Start-Process -FilePath (Join-Path $gameRoot 'Binaries\Win64\LunarRescue.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru
while(-not $p.WaitForExit(1000)){
 if((Get-Date) -gt $started.AddMinutes(35)){Stop-Process -Id $p.Id;throw 'Full mission profile timed out'}
 $p.Refresh();$peak=[Math]::Max($peak,$p.WorkingSet64);$ticks++
 if($ticks%5 -eq 0){
  $gpu=$null
  if($gpuAvailable){try{$values=(Get-Counter '\GPU Process Memory(*)\Dedicated Usage' -ErrorAction Stop).CounterSamples | Where-Object {$_.InstanceName -like "pid_$($p.Id)_*"};if($values){$gpu=($values | Measure-Object CookedValue -Sum).Sum;$gpuPeak=[Math]::Max([double]$gpuPeak,$gpu)}}catch{$gpuAvailable=$false}}
  $samples+=[ordered]@{elapsed_seconds=((Get-Date)-$started).TotalSeconds;working_set_bytes=$p.WorkingSet64;dedicated_gpu_bytes=$gpu}
 }
}
} finally {
 if($configExisted){New-Item -ItemType Directory -Force -Path (Split-Path $configPath) | Out-Null;[System.IO.File]::WriteAllBytes($configPath,$configBytes)}
 elseif(Test-Path -LiteralPath $configPath){Remove-Item -LiteralPath $configPath}
}
$csv=Get-ChildItem -LiteralPath (Join-Path $gameRoot 'Saved\Profiling\CSV') -Filter '*.csv' | Where-Object {$_.LastWriteTime -ge $started} | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$logText=Get-Content -LiteralPath $log -Raw
$result=Get-Content (Join-Path $gameRoot 'Reports\apollo12-walkthrough.txt') -Raw
if(-not $csv -or $logText -notmatch 'Capture Ended' -or $logText -match 'Fatal error:|Unhandled Exception:' -or $result -notmatch 'RESULT: 0 failures'){throw "No complete mission/capture evidence; see $log"}
Copy-Item -LiteralPath $csv.FullName -Destination (Join-Path $report 'capture.csv')
Copy-Item (Join-Path $gameRoot 'Reports\apollo12-walkthrough.txt') (Join-Path $report 'mission-result.txt')
python (Join-Path $PSScriptRoot 'summarize_profile.py') (Join-Path $report 'capture.csv') --expected-frames 0 --full-mission --output (Join-Path $report 'summary.json')
if($LASTEXITCODE -ne 0){throw 'Invalid capture'}
$summary=Get-Content (Join-Path $report 'summary.json') -Raw | ConvertFrom-Json
if($result -notmatch '(?m)^Elapsed ([0-9.]+) s'){throw 'Missing mission elapsed time'}
$missionSeconds=[double]$Matches[1]
if($summary.sample_seconds -lt ($missionSeconds-20) -or $summary.sample_seconds -gt ($missionSeconds+60)){throw 'CSV duration does not cover the complete normal-delta mission'}
[ordered]@{utc=$started.ToUniversalTime().ToString('o');cpu=(Get-CimInstance Win32_Processor).Name;adapters=@((Get-CimInstance Win32_VideoController).Name);ram_bytes=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory;process_peak_sampled_working_set_bytes=$peak;process_peak_sampled_dedicated_gpu_bytes=$gpuPeak;gpu_memory_counter_available=$gpuAvailable;quality=$Quality;resolution='1280x720';renderer='DX11 rendered offscreen';fps_cap=60;arguments=$arguments;exit_code=$p.ExitCode} | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $report 'environment.json') -Encoding UTF8
$samples | ConvertTo-Json | Set-Content (Join-Path $report 'memory-samples.json') -Encoding UTF8
Write-Output "Full rendered mission captured: $report"
