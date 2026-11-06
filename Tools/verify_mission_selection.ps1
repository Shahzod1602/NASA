param(
 [switch]$Packaged,
 [switch]$Review,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$editor=if($Review){'UnrealEditor.exe'}else{'UnrealEditor-Cmd.exe'}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{Join-Path 'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64' $editor}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$reports=Join-Path $root 'Reports\MissionSelection20261031'
New-Item -ItemType Directory -Force -Path $reports | Out-Null
$prefix=if($Packaged){'package'}else{'editor'}
$kind=if($Review){'review'}else{'test'}
$suffix="$prefix-$kind-${Width}x${Height}-$(Get-Date -Format 'yyyyMMdd-HHmmss-fff')"
$log=Join-Path $reports "$suffix.log"
$flag=if($Review){'-MissionSelectionReview'}else{'-MissionSelectionTest'}
$arguments=@('/Game/Rescue/Maps/MissionSelect','-game','-unattended','-nosound',$flag,"-abslog=`"$log`"")
if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
if($Review){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
# Automation flags prevent normal saves. Preserve the engine-level settings
# file byte for byte as Unreal can still save scalability on shutdown.
$config=Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$started=[DateTime]::UtcNow
try{
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(180000)){
  # Stop only the exact process created by this invocation.
  Stop-Process -Id $process.Id
  $process.WaitForExit()
  throw "Mission selection validation timed out; see $log"
 }
 if($process.ExitCode -ne 0){throw "Mission selection validation exited $($process.ExitCode); see $log"}
}finally{
 if($hadConfig){
  New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null
  [IO.File]::WriteAllBytes($config,$configBytes)
 }elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
if(-not(Test-Path -LiteralPath $log)){throw "Missing validation log: $log"}
$runLog=Get-Content -LiteralPath $log -Raw
if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag'){throw "Mission selection run has a crash or unsupported material; see $log"}
if($Review){
 if($runLog -match "MISSION_SELECTION_REVIEW FAILED"){throw "Rendered mission selection review failed; see $log"}
 $source=Join-Path $projectRoot 'Reports\MissionSelection20261031\review'
 $manifestFile=Join-Path $source 'review-manifest.json'
 if(-not(Test-Path -LiteralPath $manifestFile) -or (Get-Item -LiteralPath $manifestFile).LastWriteTimeUtc -lt $started){throw "Missing fresh review manifest: $manifestFile"}
 $manifest=Get-Content -LiteralPath $manifestFile -Raw | ConvertFrom-Json
 if(-not $manifest.synthetic -or $manifest.human_playtest -or $manifest.performance_measurement){throw 'Review must declare synthetic presentation samples without human or performance claims'}
 $names=@('home','moon-unselected','moon-apollo-selected','moon-selene-selected','mars-unselected','mars-ares-selected','moon150-selected','actions-focus','actions-hover','actions-pressed','actions-idle')+@(0..5 | ForEach-Object {'transition-{0:D2}' -f $_})
 if(@($manifest.frames).Count -ne $names.Count){throw 'Unexpected review frame count'}
 if(@($manifest.pointer_clicks).Count -ne 3){throw 'Missing three actual Slate pointer-route checks'}
 if(@($manifest.action_checks).Count -ne 6){throw 'Missing action hit/focus checks at 100% and 150% text'}
 foreach($textScale in @(1.0,1.5)){foreach($actionName in @('start','back','settings')){
  $action=@($manifest.action_checks|Where-Object { $_.name -eq $actionName -and [Math]::Abs([double]$_.text_scale-$textScale) -lt .001 })
  if($action.Count -ne 1 -or -not $action[0].full_fill_and_centred_caption -or -not $action[0].five_hit_points -or -not $action[0].actual_keyboard_focus){throw "Action surface/geometry/keyboard focus failed: $actionName at text scale$textScale"}
 }}

 foreach($index in 0..2){
  $click=@($manifest.pointer_clicks | Where-Object mission_index -EQ $index)
  if($click.Count -ne 1 -or -not $click[0].button_in_hit_path -or -not $click[0].down_handled -or -not $click[0].up_handled -or -not $click[0].selected){throw "Actual description-region Slate click did not select mission $index"}
 }

 $destination=Join-Path $reports $suffix
 New-Item -ItemType Directory -Path $destination | Out-Null
 foreach($name in $names){
  $frame=@($manifest.frames | Where-Object name -EQ $name)
  if($frame.Count -ne 1){throw "Missing or duplicate frame measurement: $name"}
  if(@($frame[0].mission_blends).Count -ne 3 -or @($frame[0].home_blends).Count -ne 3){throw "Incomplete blend measurements: $name"}
  foreach($blend in @($frame[0].mission_blends)+@($frame[0].home_blends)){
   if([double]::IsNaN([double]$blend) -or [double]::IsInfinity([double]$blend) -or $blend -lt 0 -or $blend -gt 1){throw "Invalid blend measurement: $name"}
  }
  $file=Join-Path $source "$name.png"
  if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTimeUtc -lt $started){throw "Missing fresh review PNG: $file"}
  $bytes=[IO.File]::ReadAllBytes($file)
  if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $file"}
  $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16))
  $h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
  if($w -ne $Width -or $h -ne $Height){throw "Unexpected PNG dimensions ${w}x${h}: $file"}
  Copy-Item -LiteralPath $file -Destination $destination
 }
 foreach($expected in @(@('moon-unselected',-1),@('moon-apollo-selected',0),@('moon-selene-selected',1),@('mars-unselected',-1),@('mars-ares-selected',2),@('moon150-selected',0))){
  $frame=$manifest.frames | Where-Object name -EQ $expected[0]
  if(-not $frame.mission_selection -or $frame.selected_mission -ne $expected[1]){throw "Incorrect rendered mission selection fixture: $($expected[0])"}
  # Collapsed cards do not tick, so their cached (invisible) transition may
  # retain an old value. Assert exclusive settled highlight on visible cards.
  $visibleIndices=if($expected[0] -like 'mars-*'){@(2)}else{@(0,1)}
  foreach($index in $visibleIndices){
   $expectedBlend=if($expected[1] -eq $index){1.0}else{0.0}
   if([Math]::Abs([double]$frame.mission_blends[$index]-$expectedBlend) -gt 0.001){throw "Unsettled or incorrect selected highlight in $($expected[0])"}
  }
 }
 if(($manifest.frames | Where-Object name -EQ 'moon150-selected').text_scale -ne 1.5){throw '150% text fixture was not active'}
 foreach($expected in @(@('moon-apollo-selected',0),@('moon-selene-selected',1),@('mars-ares-selected',2),@('moon150-selected',0))){
  $frame=$manifest.frames | Where-Object name -EQ $expected[0]
  if([Math]::Abs([double]$frame.mission_blends[$expected[1]]-1) -gt 0.001){throw "Selected blue state did not settle: $($expected[0])"}
 }
 $previous=-1.0;$intermediate=0
 foreach($name in @(0..5 | ForEach-Object {'transition-{0:D2}' -f $_})){
  $frame=$manifest.frames | Where-Object name -EQ $name
  $value=[double]$frame.mission_blends[0]
  if($value -lt $previous){throw 'Rendered selection blend moved backwards'}
  if($value -gt 0 -and $value -lt 1){$intermediate++}
  $previous=$value
 }
 if($intermediate -eq 0){throw 'Rendered review did not sample an intermediate animation state'}

 Copy-Item -LiteralPath $manifestFile -Destination $destination
 if($runLog -notmatch 'MISSION_SELECTION_REVIEW COMPLETE'){throw 'Review did not report successful completion'}
 Write-Output "PASS: eleven menu/action states, six naturally ticking transition samples and six action hit/focus checks at ${Width}x${Height}: $destination"
 Write-Output 'These are staged visual samples, not a human playtest or performance benchmark.'
}else{
 $result=Join-Path $projectRoot 'Reports\mission-selection-checks.txt'
 if(-not(Test-Path -LiteralPath $result) -or (Get-Item -LiteralPath $result).LastWriteTimeUtc -lt $started){throw "Missing fresh mission selection result: $result"}
 $text=Get-Content -LiteralPath $result -Raw
 if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Mission selection assertions failed; see $result"}
 Copy-Item -LiteralPath $result -Destination (Join-Path $reports "$suffix-checks.txt")
 Write-Output $text
}
Write-Output "Mission selection evidence log: $log"
