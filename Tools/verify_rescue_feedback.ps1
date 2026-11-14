param([switch]$Packaged)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$project=if($Packaged){Join-Path $root 'Dist/Apollo12/Windows/LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $project 'Binaries/Win64/LunarRescue.exe'}else{'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'}
$kind=if($Packaged){'package'}else{'editor'}
$destination=Join-Path $root ("Reports/Feedback20261114Evening/$kind-rescue-"+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $destination|Out-Null
$config=Join-Path $project ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$bytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$hash=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
$cases=@(
 @{Name='selene-primary';Map='/Game/Lunar/Maps/MoonBase';Flag='LunarTest';Report='gameplay-checks.txt'},
 @{Name='mars-primary';Map='/Game/Mars/Maps/MarsBase';Flag='LunarTest';Report='mars-gameplay-checks.txt'},
 @{Name='selene-recorder';Map='/Game/Lunar/Maps/MoonBase';Flag='LunarLedgeTest';Report='ledge-checks.txt'},
 @{Name='selene-recorder-fallback';Map='/Game/Lunar/Maps/MoonBase';Flag='LunarLedgeTest';Report='ledge-checks.txt';Extra='-LunarMissingLedgeMesh'},
 @{Name='mars-movement';Map='/Game/Mars/Maps/MarsBase';Flag='LunarMovementTest';Report='mars-movement-checks.txt'},
 @{Name='mars-recorder';Map='/Game/Mars/Maps/MarsBase';Flag='LunarLedgeTest';Report='mars-ledge-checks.txt'}
)
$results=[Collections.Generic.List[object]]::new();$completed=$false
try{
 foreach($case in $cases){
  $log=Join-Path $destination ($case.Name+'.log')
  $arguments=@($case.Map,'-game','-unattended','-nullrhi','-nosound',('-'+$case.Flag),"-abslog=`"$log`"")
  if($case.Extra){$arguments+=$case.Extra}
  if(-not $Packaged){$arguments=@("`"$root/LunarRescue.uproject`"")+$arguments}
  $started=[DateTime]::UtcNow;$process=$null
  try{
   $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
   if(-not $process.WaitForExit(180000)){throw "$($case.Name) exceeded its bounded test deadline"}
   if($process.ExitCode -ne 0){throw "$($case.Name) exited $($process.ExitCode); see $log"}
  }finally{if($process -and -not $process.HasExited){Stop-Process -Id $process.Id;[void]$process.WaitForExit(5000)}}
  $report=Join-Path $project ('Reports/'+$case.Report)
  if(-not(Test-Path -LiteralPath $report) -or (Get-Item -LiteralPath $report).LastWriteTimeUtc -lt $started){throw "Missing fresh report: $report"}
  $text=Get-Content -LiteralPath $report -Raw;$count=[regex]::Matches($text,'(?m)^PASS ').Count
  if($count -eq 0 -or $text -match '(?m)^FAIL ' -or [regex]::Matches($text,'(?m)^RESULT: 0 failures\r?$').Count -ne 1){throw "Failed rescue assertions: $report"}
  $runLog=Get-Content -LiteralPath $log -Raw
  if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:'){throw "Crash in $log"}
  if($case.Extra -and $runLog -notmatch 'Lunar ledge mesh unavailable or degenerate; using safe fallback'){throw 'Fallback case did not exercise the missing-mesh branch'}
  if((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant() -ne $hash){throw 'Executable changed during validation'}
  Copy-Item -LiteralPath $report -Destination (Join-Path $destination ($case.Name+'-checks.txt'))
  $results.Add([pscustomobject]@{name=$case.Name;assertions=$count;passed=$true;started_utc=$started.ToString('o')})
  Write-Output "PASS $($case.Name): $count assertions"
 }
 $completed=$true
}finally{
 if($hadConfig){New-Item -ItemType Directory -Path (Split-Path $config -Parent) -Force|Out-Null;[IO.File]::WriteAllBytes($config,$bytes)}elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
 [ordered]@{completed=$completed;packaged=[bool]$Packaged;executable_sha256=$hash;cases=$results;human_playtest=$false;graphics_test=$false;performance_measurement=$false;completed_utc=[DateTime]::UtcNow.ToString('o')}|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $destination 'run-summary.json') -Encoding utf8
}
Write-Output "Rescue feedback evidence: $destination"
