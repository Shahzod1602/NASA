param(
 [switch]$Packaged,
 [switch]$Review,
 [switch]$Intro,
 [switch]$UI,
 [switch]$Pov,
 [switch]$Learning,
 [switch]$Restart,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$editor=if($Review){'UnrealEditor.exe'}else{'UnrealEditor-Cmd.exe'}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{Join-Path 'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64' $editor}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
if(-not($Intro -or $UI -or $Pov -or $Learning -or $Restart)){
 $Intro=$true;$UI=$true;$Pov=$true
 if(-not $Review){$Learning=$true;$Restart=$true}
}
if($Review -and $Restart){throw '-Restart is a functional travel check; run it without -Review'}
$maps=@(
 [pscustomobject]@{Label='apollo';Map='/Game/Lunar/Maps/Apollo12';Prefix=''},
 [pscustomobject]@{Label='selene';Map='/Game/Lunar/Maps/MoonBase';Prefix=''},
 [pscustomobject]@{Label='mars';Map='/Game/Mars/Maps/MarsBase';Prefix='mars-'}
)
$cases=[Collections.Generic.List[object]]::new()
if($Review){
 if($Intro){$cases.Add([pscustomobject]@{Name='intro';Map='/Game/Rescue/Maps/MissionSelect';Flag='NasaUIReview';ReviewFolder='ui-intro';Manifest='ui-manifest.json';Expected=@('intro-fade-in','intro-illustration','intro-menu-reveal','home-five-glass-actions','home-settings-hover','home-exit-hover','home-mars-hover')})}
 if($UI -or $Learning){foreach($map in $maps){
  $guide=if($map.Label -eq 'apollo'){'apollo-photographed-sample-guide'}else{'terminal-actual-hold'}
  $uiFrames=@('mission-compact-hud',$guide,'pause-five-actions','pause-restart-confirmation','pause-settings','pause-settings-return','field-lesson-question','field-lesson-explanation','field-lesson-alternative','field-lesson-text150','pause-text150','debrief-session-lessons')
  if($map.Label -eq 'apollo'){$uiFrames+='debrief-win-lessons-text150'}
  $cases.Add([pscustomobject]@{Name="ui-$($map.Label)";Map=$map.Map;Flag='NasaUIReview';ReviewFolder="ui-$($map.Label)";Manifest='ui-manifest.json';Expected=$uiFrames})
 }}
 if($Pov){
  $moonFrames=@('moon-lookdown-fov70','moon-lookdown-fov85','moon-lookdown-fov110','moon-free-hands','soil-step2-fov85','soil-step2-fov70','soil-step2-fov110','soil-dig-motion','soil-lift-motion','soil-deposit','soil-seal','soil-lower','cargo-approach','cargo-middle','cargo-settled')
  $marsFrames=@('mars-cell-carry','mars-station-restart','mars-science-download','mars-relay-alignment','mars-recorder','mars-free-lookdown')
  $cases.Add([pscustomobject]@{Name='pov-moon';Map=$maps[0].Map;Flag='NasaPovReview';ReviewFolder='pov-moon';Manifest='pov-manifest.json';Expected=$moonFrames})
  $cases.Add([pscustomobject]@{Name='pov-mars';Map=$maps[2].Map;Flag='NasaPovReview';ReviewFolder='pov-mars';Manifest='pov-manifest.json';Expected=$marsFrames})
 }
}else{
 if($Intro){$cases.Add([pscustomobject]@{Name='intro';Map='/Game/Rescue/Maps/MissionSelect';Flag='NasaIntroTest';Report='nasa-intro-checks.txt'})}
 if($UI){foreach($map in $maps){$cases.Add([pscustomobject]@{Name="ui-$($map.Label)";Map=$map.Map;Flag='NasaUITest';Report="$($map.Prefix)nasa-ui-checks.txt"})}}
 if($Pov){$cases.Add([pscustomobject]@{Name='pov';Map=$maps[0].Map;Flag='NasaPovTest';Report='pov-checks.txt'})}
 if($Learning){foreach($map in $maps){$cases.Add([pscustomobject]@{Name="learning-$($map.Label)";Map=$map.Map;Flag='NasaLearningTest';Report='field-learning-checks.txt'})}}
 if($Restart){foreach($map in $maps){$cases.Add([pscustomobject]@{Name="restart-$($map.Label)";Map=$map.Map;Flag='NasaRestartTest';Report="$($map.Prefix)nasa-restart-checks.txt"})}}
}
if(-not $cases.Count){throw 'No validation cases were selected'}
$reports=Join-Path $root 'Reports\NasaTasks20261101'
$prefix=if($Packaged){'package'}else{'editor'}
$kind=if($Review){'review'}else{'test'}
$destination=Join-Path $reports "$prefix-$kind-${Width}x${Height}-$(Get-Date -Format 'yyyyMMdd-HHmmss-fff')"
New-Item -ItemType Directory -Path $destination -Force | Out-Null
$config=Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$results=[Collections.Generic.List[object]]::new()
foreach($case in $cases){
 $log=Join-Path $destination "$($case.Name).log"
 $arguments=@($case.Map,'-game','-unattended','-nosound',"-$($case.Flag)","-abslog=`"$log`"")
 if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
 if($Review){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
 # Each case begins with the original engine settings. The test/review flags also
 # disable normal game saves; no playtest participant or player photo is created.
 $hadConfig=Test-Path -LiteralPath $config
 $configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
 $started=[DateTime]::UtcNow
 try{
  $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
  if(-not $process.WaitForExit(180000)){
   if(-not $process.HasExited){Stop-Process -Id $process.Id -ErrorAction SilentlyContinue}
   $process.WaitForExit()
   throw "$($case.Name) timed out; see $log"
  }
  if($process.ExitCode -ne 0){throw "$($case.Name) exited $($process.ExitCode); see $log"}
 }finally{
  if($hadConfig){
   New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null
   [IO.File]::WriteAllBytes($config,$configBytes)
  }elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
 }
 if(-not(Test-Path -LiteralPath $log)){throw "Missing log: $log"}
 $runLog=Get-Content -LiteralPath $log -Raw
 if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag|NASA_POV_REVIEW FAILED|NASA_UI_REVIEW FAILED'){throw "$($case.Name) has a crash or rendering failure; see $log"}
 if($Review){
  $complete=if($case.Flag -eq 'NasaPovReview'){'NASA_POV_REVIEW COMPLETE'}else{'NASA_UI_REVIEW COMPLETE'}
  if($runLog -notmatch [regex]::Escape($complete)){throw "Missing successful review completion: $log"}
  $source=Join-Path (Join-Path $projectRoot 'Reports\NasaTasks20261101') $case.ReviewFolder
  $manifestPath=Join-Path $source $case.Manifest
  if(-not(Test-Path -LiteralPath $manifestPath) -or (Get-Item -LiteralPath $manifestPath).LastWriteTimeUtc -lt $started){throw "Missing fresh manifest: $manifestPath"}
  $manifest=Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
  if(-not $manifest.synthetic -or $manifest.human_playtest -or $manifest.performance_measurement){throw 'Review must declare staged visual samples without human or performance claims'}
  if($case.Flag -eq 'NasaUIReview' -and $case.Name -ne 'intro'){
   foreach($action in @('pause-restart','pause-cancel-restart','pause-settings','pause-resume')){
    $click=@($manifest.pointer_clicks | Where-Object action -EQ $action)
    if($click.Count -ne 1 -or -not $click[0].button_in_hit_path -or -not $click[0].down_handled -or -not $click[0].up_handled){throw "Missing actual Slate pause pointer check: $action"}
    if($action -eq 'pause-restart' -and -not $click[0].confirmation_after){throw 'Slate Restart click did not open confirmation'}
    if($action -eq 'pause-cancel-restart' -and ($click[0].confirmation_after -or -not $click[0].paused_after)){throw 'Slate Cancel click did not preserve pause'}
    if($action -eq 'pause-settings' -and (-not $click[0].settings_after -or -not $click[0].paused_after)){throw 'Slate Settings click did not open settings while paused'}
    if($action -eq 'pause-resume' -and $click[0].paused_after){throw 'Slate Resume click did not resume'}
   }
  }
  $frames=@($manifest.frames)
  if($frames.Count -eq 0){throw "Empty visual review: $manifestPath"}
  if($case.Expected.Count -gt 0 -and $frames.Count -ne $case.Expected.Count){throw "Unexpected $($case.Name) frame count"}
  $copied=Join-Path $destination $case.Name;New-Item -ItemType Directory -Path $copied | Out-Null
  foreach($frame in $frames){
   $name=[string]$frame.name
   if($name -notmatch '^[a-z0-9][a-z0-9-]*$' -or @($frames | Where-Object name -EQ $name).Count -ne 1){throw "Invalid or duplicate frame name: $name"}
   if($case.Expected.Count -gt 0 -and $name -notin $case.Expected){throw "Unexpected frame: $name"}
   $file=Join-Path $source "$name.png"
   if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTimeUtc -lt $started){throw "Missing fresh PNG: $file"}
   $bytes=[IO.File]::ReadAllBytes($file)
   if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $file"}
   $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
   if($w -ne $Width -or $h -ne $Height){throw "Unexpected PNG dimensions ${w}x${h}: $file"}
   if($name -like 'home-*-hover' -and -not $frame.menu_hover_fixture){throw "Menu hover image must declare its scripted fixture: $name"}
   if($name -in @('home-settings-hover','home-exit-hover') -and (Get-FileHash -LiteralPath $file).Hash -eq (Get-FileHash -LiteralPath (Join-Path $source 'home-five-glass-actions.png')).Hash){throw "Hover style was not rendered differently from its base: $name"}
   if($name -like '*lookdown*' -and $frame.hands_visible){throw "Free look-down unexpectedly displays hands: $name"}
   if($name -like 'soil-step2-*' -and (-not $frame.hands_visible -or $frame.leg_proxies_visible)){throw "Soil step 2/2 has hidden task hands or visible near-cut leg proxies: $name"}
   if($name -eq 'terminal-actual-hold' -and ([Math]::Abs([double]$frame.hold_fraction-.65) -gt .001)){throw 'Rendered terminal hold must use the actual 65% gameplay progress'}
   if($name -like 'pause-*' -and (-not $frame.paused -or $frame.pause_actions -ne 5)){throw "Pause fixture does not show the five paused actions: $name"}
   if($name -eq 'pause-restart-confirmation' -and -not $frame.restart_confirmation){throw 'Restart confirmation fixture was not active'}
   if($name -like 'field-lesson-*' -and -not $frame.field_lessons_open){throw "Field lesson was not actually open: $name"}
   if($name -eq 'field-lesson-explanation' -and ($frame.lesson_reviewed -ne 1 -or $frame.lesson_matching_choices -ne 1)){throw 'Scripted correct lesson response was not recorded'}
   if($name -like 'debrief-*' -and ($frame.paused -or $frame.lesson_reviewed -ne 2 -or $frame.lesson_matching_choices -ne 1)){throw 'Debrief fixture must use its actual two latest session answers'}
   if($name -eq 'field-lesson-alternative' -and ($frame.lesson_reviewed -ne 2 -or $frame.lesson_matching_choices -ne 1)){throw 'Scripted alternative lesson response was not recorded independently'}
   if($name -like '*text150' -and $frame.text_scale -ne 1.5){throw "150% text scale was not active: $name"}
   Copy-Item -LiteralPath $file -Destination $copied
  }
  Copy-Item -LiteralPath $manifestPath -Destination $copied
  $results.Add([pscustomobject]@{Case=$case.Name;Flag=$case.Flag;Map=$case.Map;Frames=$frames.Count;Outcome='passed';Synthetic=$true;HumanPlaytest=$false;PerformanceMeasurement=$false})
  Write-Output "PASS $($case.Name): $($frames.Count) fresh staged PNGs at ${Width}x${Height}"
 }else{
  $result=Join-Path (Join-Path $projectRoot 'Reports') $case.Report
  if(-not(Test-Path -LiteralPath $result) -or (Get-Item -LiteralPath $result).LastWriteTimeUtc -lt $started){throw "Missing fresh result: $result"}
  $text=Get-Content -LiteralPath $result -Raw
  if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Failed assertions; see $result"}
  $count=[regex]::Matches($text,'(?m)^PASS ').Count
  if($count -eq 0){throw "No assertions found: $result"}
  Copy-Item -LiteralPath $result -Destination (Join-Path $destination "$($case.Name)-checks.txt")
  $results.Add([pscustomobject]@{Case=$case.Name;Flag=$case.Flag;Map=$case.Map;Assertions=$count;Failures=0;Outcome='passed'})
  Write-Output "PASS $($case.Name): $count assertions"
 }
}
[pscustomobject]@{Cases=$results;Executable=$exe;ExecutableSha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant();HumanPlaytest=$false;Review=[bool]$Review;Width=$Width;Height=$Height;CompletedAtUtc=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $destination 'run-summary.json') -Encoding utf8
Write-Output "NASA task validation evidence: $destination"
