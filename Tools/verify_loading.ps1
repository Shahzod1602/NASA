param(
 [switch]$Packaged,
 [switch]$Graphics,
 [switch]$Onscreen,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
if($Onscreen -and -not $Graphics){throw '-Onscreen requires -Graphics'}
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$reports=Join-Path $root 'Reports\Loading20261029'
New-Item -ItemType Directory -Force -Path $reports | Out-Null
$prefix=if($Packaged){'package'}else{'editor'}
$mode=if($Onscreen){'onscreen'}elseif($Graphics){'offscreen'}else{'nullrhi'}
$suffix="$prefix-$mode-${Width}x${Height}-$(Get-Date -Format 'yyyyMMdd-HHmmss-fff')"
$log=Join-Path $reports "$suffix.log"
$arguments=@('/Game/Rescue/Maps/MissionSelect','-game','-LoadingTest','-nosound',"-abslog=`"$log`"")
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
Write-Output $text
Write-Output "Loading validation evidence: $log"
