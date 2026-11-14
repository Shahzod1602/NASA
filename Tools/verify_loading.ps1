param(
 [switch]$Packaged,
 [switch]$Graphics,
 [switch]$Onscreen,
 [switch]$Intro,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
if($Onscreen -and -not $Graphics){throw '-Onscreen requires -Graphics'}
if($Intro -and -not $Graphics){throw '-Intro requires a real rendered -Graphics run'}
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$reports=Join-Path $root 'Reports\Loading20261029'
New-Item -ItemType Directory -Force -Path $reports | Out-Null
$prefix=if($Packaged){'package'}else{'editor'}
$mode=if($Intro -and $Onscreen){'intro-onscreen'}elseif($Intro){'intro-offscreen'}elseif($Onscreen){'onscreen'}elseif($Graphics){'offscreen'}else{'nullrhi'}
$suffix="$prefix-$mode-${Width}x${Height}-$(Get-Date -Format 'yyyyMMdd-HHmmss-fff')"
$log=Join-Path $reports "$suffix.log"
$arguments=@('/Game/Rescue/Maps/MissionSelect','-game','-LoadingTest','-nosound',"-abslog=`"$log`"")
if($Intro){$arguments=@($arguments|Where-Object {$_ -ne '-nosound'})+'-ReviewMissionIntro'}
if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
if($Graphics){
 $arguments+=@('-LoadingRenderedTest','-dx11','-ForceRes',"-ResX=$Width","-ResY=$Height")
 if($Onscreen){$arguments+='-windowed'}else{$arguments+='-RenderOffscreen'}
}else{$arguments+=@('-nullrhi','-unattended')}
# LoadingTest writes its checkpoint under Saved/AutomatedTests/Loading.
# Unreal can still write scalability on shutdown, so restore those bytes too.
$config=Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$introMovie=Join-Path $projectRoot 'Content/Movies/MoonArrival.mp4'
$introHash=$null
if($Intro){
 if(-not(Test-Path -LiteralPath $introMovie -PathType Leaf)){throw '-Intro needs the optional local MoonArrival.mp4; missing-media fallback is a separate case'}
 $introHash=(Get-FileHash -LiteralPath $introMovie -Algorithm SHA256).Hash.ToLowerInvariant()
 $expected=(Get-Content -LiteralPath (Join-Path $root 'SourceArt/MissionIntro20261114/manifest.json') -Raw|ConvertFrom-Json).sha256
 if($introHash -ne $expected){throw 'Intro binary differs from the recorded unmodified local reference'}
}
$started=Get-Date
try {
 $style=if($Onscreen){'Normal'}else{'Hidden'}
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle $style -PassThru
 if(-not $process.WaitForExit(240000)){
  Stop-Process -Id $process.Id
  $process.WaitForExit()
  throw "Loading validation timed out; see $log"
 }
 if($process.ExitCode -ne 0){throw "Loading validation exited $($process.ExitCode); see $log"}
}finally{
 if($hadConfig){
  New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null
  [IO.File]::WriteAllBytes($config,$configBytes)
 }elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
$result=Join-Path $projectRoot 'Reports\loading-checks.txt'
if(-not(Test-Path -LiteralPath $result) -or (Get-Item -LiteralPath $result).LastWriteTime -lt $started){throw "Missing fresh loading result: $result"}
$text=Get-Content -LiteralPath $result -Raw
if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Loading assertions failed; see $result"}
Copy-Item -LiteralPath $result -Destination (Join-Path $reports "$suffix-checks.txt")
if($Intro){
 $runLog=Get-Content -LiteralPath $log -Raw
 $begins=[regex]::Matches($runLog,'MISSION_INTRO begin map=(\w+) started=1 movie=MoonArrival')
 $ends=[regex]::Matches($runLog,'MISSION_INTRO end elapsed=([0-9.]+) clip_finished=1 slate_ticks=(\d+) game_ticks=(\d+) world_time_held=1')
 if($begins.Count -ne 2 -or $begins[0].Groups[1].Value -ne 'Apollo12' -or $begins[1].Groups[1].Value -ne 'MoonBase' -or $ends.Count -ne 2){throw 'Both selected Moon clips must actually finish; no resume/restart/Mars clip may play'}
 $durations=@();foreach($end in $ends){$seconds=[double]::Parse($end.Groups[1].Value,[Globalization.CultureInfo]::InvariantCulture);if($seconds -lt 29.5 -or $seconds -gt 35 -or [int]$end.Groups[3].Value -le 0){throw 'A short skip, timeout or non-ticking clip cannot count as playback success'};$durations+=$seconds}
 if((Get-FileHash -LiteralPath $introMovie -Algorithm SHA256).Hash.ToLowerInvariant() -ne $introHash){throw 'Intro binary changed during review'}
 [ordered]@{actual_selected_travel=$true;movies=@('Apollo12','MoonBase');durations_seconds=$durations;world_time_held=$true;original_media_sha256=$introHash;audio_enabled=$true;audible_human_review=$false;human_playtest=$false;performance_measurement=$false;source_redistribution_permission='unconfirmed';log=$log}|ConvertTo-Json -Depth 4|Set-Content -LiteralPath (Join-Path $reports "$suffix-intro.json") -Encoding utf8
 Write-Output ('PASS two complete local intro clips with held world clocks: '+($durations -join ', ')+' seconds')
}
Write-Output $text
Write-Output "Loading validation evidence: $log"
