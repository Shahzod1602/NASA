param(
 [switch]$Packaged,
 [switch]$Review,
 [ValidateSet("all","apollo","selene","mars")][string]$Mission="all",
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist/Apollo12/Windows/LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries/Win64/LunarRescue.exe'}elseif($Review){'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe'}else{'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$maps=@(
 [pscustomobject]@{Name='apollo';Map='/Game/Lunar/Maps/Apollo12';Prefix='';Frames=9},
 [pscustomobject]@{Name='selene';Map='/Game/Lunar/Maps/MoonBase';Prefix='';Frames=8},
 [pscustomobject]@{Name='mars';Map='/Game/Mars/Maps/MarsBase';Prefix='mars-';Frames=8}
)
if($Mission -ne 'all'){$maps=@($maps | Where-Object Name -EQ $Mission)}
$kind=if($Review){'review'}else{'test'};$build=if($Packaged){'package'}else{'editor'}
$destination=Join-Path $root ('Reports/Feedback20261102/'+$build+'-'+$kind+"-${Width}x${Height}-"+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $destination | Out-Null
$cases=[Collections.Generic.List[object]]::new()
foreach($map in $maps){
 if($Review){$cases.Add([pscustomobject]@{Name="hud-$($map.Name)";Map=$map.Map;Flag='HUDReferenceReview';Report="Feedback20261102/hud-$($map.Name)/hud-reference-checks.txt";Folder="Feedback20261102/hud-$($map.Name)";Frames=$map.Frames})}
 else{
  $cases.Add([pscustomobject]@{Name="assets-$($map.Name)";Map=$map.Map;Flag='FeedbackAssetsTest';Report="$($map.Prefix)feedback-assets-checks.txt"})
  $cases.Add([pscustomobject]@{Name="diagnostics-$($map.Name)";Map=$map.Map;Flag='EquipmentDiagnosticsTest';Report="$($map.Prefix)equipment-diagnostics-checks.txt"})
 }
}
$results=[Collections.Generic.List[object]]::new()
foreach($case in $cases){
 $log=Join-Path $destination "$($case.Name).log"
 $arguments=@($case.Map,'-game','-unattended','-nosound',"-$($case.Flag)","-abslog=`"$log`"")
 if(-not $Packaged){$arguments=@("`"$root/LunarRescue.uproject`"")+$arguments}
 if($Review){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
 $config=Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
 $hadConfig=Test-Path -LiteralPath $config -PathType Leaf
 $configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
 $started=[DateTime]::UtcNow
 try{
  $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
  if(-not $process.WaitForExit(180000)){if(-not $process.HasExited){Stop-Process -Id $process.Id};$process.WaitForExit();throw "Timeout: $($case.Name); see $log"}
  if($process.ExitCode -ne 0){throw "$($case.Name) exited $($process.ExitCode); see $log"}
 }finally{
  if($hadConfig){New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null;[IO.File]::WriteAllBytes($config,$configBytes)}
  elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
 }
 $logText=Get-Content -LiteralPath $log -Raw
 if($logText -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag'){throw "Crash/render failure: $log"}
 $report=Join-Path $projectRoot ('Reports/'+$case.Report)
 if(-not(Test-Path -LiteralPath $report) -or (Get-Item -LiteralPath $report).LastWriteTimeUtc -lt $started){throw "No fresh check report: $report"}
 $text=Get-Content -LiteralPath $report -Raw
 if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Assertion failure: $report"}
 $count=[regex]::Matches($text,'(?m)^PASS ').Count
 if($count -eq 0){throw "Empty assertions: $report"}
 Copy-Item -LiteralPath $report -Destination (Join-Path $destination "$($case.Name)-checks.txt")
 $frames=0
 if($Review){
  if($logText -notmatch 'HUD_REFERENCE_REVIEW COMPLETE'){throw "No rendered completion: $log"}
  $source=Join-Path $projectRoot ('Reports/'+$case.Folder);$manifestPath=Join-Path $source 'hud-reference-manifest.json'
  if(-not(Test-Path -LiteralPath $manifestPath) -or (Get-Item -LiteralPath $manifestPath).LastWriteTimeUtc -lt $started){throw 'No fresh HUD manifest'}
  $manifest=Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
  if(-not $manifest.synthetic -or $manifest.human_playtest -or $manifest.performance_measurement){throw 'Invalid fixture provenance'}
  $frames=@($manifest.frames).Count;if($frames -ne $case.Frames -or $count -ne $frames){throw 'Incorrect HUD frame/check count'}
  $imageFolder=Join-Path $destination $case.Name;New-Item -ItemType Directory -Path $imageFolder | Out-Null
  foreach($frame in $manifest.frames){
   if(-not $frame.layout_passed -or $frame.name -notmatch '^[a-z0-9-]+$'){throw 'Invalid layout frame'}
   $file=Join-Path $source "$($frame.name).png"
   if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTimeUtc -lt $started){throw "No fresh PNG: $file"}
   $bytes=[IO.File]::ReadAllBytes($file);if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $file"}
   $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
   if($w -ne $Width -or $h -ne $Height){throw "Unexpected PNG size ${w}x${h}: $file"}
   if($frame.name -like '*150' -and ($frame.text_scale -ne 1.5 -or $frame.subtitle_scale -ne 1.5)){throw 'Large text fixture was not active'}
   Copy-Item -LiteralPath $file -Destination $imageFolder
  }
  Copy-Item -LiteralPath $manifestPath -Destination $imageFolder
 }
 $results.Add([pscustomobject]@{Case=$case.Name;Map=$case.Map;Flag=$case.Flag;Assertions=$count;Failures=0;Outcome='passed';Frames=$frames})
 Write-Output "PASS $($case.Name): $count checks, $frames staged frames"
}
[pscustomobject]@{Cases=$results;ExecutableSha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant();HumanPlaytest=$false;SyntheticReview=[bool]$Review;PerformanceMeasurement=$false;Width=$Width;Height=$Height;CompletedUtc=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $destination 'run-summary.json') -Encoding utf8
Write-Output "Feedback validation: $destination"