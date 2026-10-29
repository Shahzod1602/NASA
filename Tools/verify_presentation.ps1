param(
 [switch]$Packaged,
 [switch]$Graphics,
 [switch]$Audio,
 [switch]$Review,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
if($Review){$Graphics=$true}
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$reports=Join-Path $root 'Reports\ExpeditionPolish20261029'
New-Item -ItemType Directory -Force -Path $reports | Out-Null
$prefix=if($Packaged){'package'}else{'editor'}
$kind=if($Review){'review'}else{'test'}
$mode=if($Graphics){'dx11'}else{'nullrhi'}
$sound=if($Audio){'audio'}else{'silent'}
$suffix="$prefix-$kind-$mode-$sound-${Width}x${Height}-$(Get-Date -Format 'yyyyMMdd-HHmmss-fff')"
$log=Join-Path $reports "$suffix.log"
$flag=if($Review){'-ExpeditionPresentationReview'}else{'-ExpeditionPresentationTest'}
$arguments=@('/Game/Lunar/Maps/Apollo12','-game','-unattended',$flag,"-abslog=`"$log`"")
if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
if($Graphics){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
if($Audio){$arguments+='-PresentationAudioTest'}else{$arguments+='-nosound'}
# Test/Review flags disable normal mission saves. Preserve engine-level config
# writes as well; synthetic review fixtures never capture in-game photographs.
$config=Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$started=Get-Date
try{
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(240000)){
  Stop-Process -Id $process.Id
  $process.WaitForExit()
  throw "Presentation validation timed out; see $log"
 }
 if($process.ExitCode -ne 0){throw "Presentation validation exited $($process.ExitCode); see $log"}
}finally{
 if($hadConfig){
  New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null
  [IO.File]::WriteAllBytes($config,$configBytes)
 }elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
$renderLog=Get-Content -LiteralPath $log -Raw
if($renderLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag'){throw "Presentation render has a crash or unsupported material; see $log"}
if($Review){
 $destination=Join-Path $reports $suffix
 New-Item -ItemType Directory -Force -Path $destination | Out-Null
 foreach($name in @('intrepid-wide','intrepid-foot','regolith','tongs-lift','debrief-won','debrief-lost','result150','debrief150')){
  $file=Join-Path $projectRoot "Reports\ExpeditionPolish20261029\presentation-$name.png"
  if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTime -lt $started){throw "Missing fresh review screenshot: $file"}
  $bytes=[IO.File]::ReadAllBytes($file)
  if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $file"}
  $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16))
  $h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
  if($w -ne $Width -or $h -ne $Height){throw "Unexpected screenshot dimensions: ${w}x${h}"}
  Copy-Item -LiteralPath $file -Destination $destination
 }
 $fixture=Join-Path $projectRoot 'Reports\ExpeditionPolish20261029\presentation-fixtures.json'
 if(-not(Test-Path -LiteralPath $fixture) -or (Get-Item -LiteralPath $fixture).LastWriteTime -lt $started){throw 'Missing fresh synthetic-fixture declaration'}
 Copy-Item -LiteralPath $fixture -Destination $destination
 Write-Output "PASS: eight fresh synthetic review scenes at ${Width}x${Height}: $destination"
}else{
 $result=Join-Path $projectRoot 'Reports\presentation-checks.txt'
 if(-not(Test-Path -LiteralPath $result) -or (Get-Item -LiteralPath $result).LastWriteTime -lt $started){throw "Missing fresh presentation result: $result"}
 $text=Get-Content -LiteralPath $result -Raw
 if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Presentation assertions failed; see $result"}
 Copy-Item -LiteralPath $result -Destination (Join-Path $reports "$suffix-checks.txt")
 Write-Output $text
}
Write-Output "Presentation evidence log: $log"
