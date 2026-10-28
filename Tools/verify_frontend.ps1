param([switch]$Packaged,[switch]$Review,[int]$Width=1280,[int]$Height=720)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$reports=Join-Path $root 'Reports\Frontend20261028'
New-Item -ItemType Directory -Force -Path $reports | Out-Null
$prefix=if($Packaged){'package'}else{'editor'}
$flag=if($Review){'FrontendReview'}else{'FrontendTest'}
$log=Join-Path $reports "$prefix-$flag-${Width}x${Height}.log"
$arguments=@('/Game/Rescue/Maps/MissionSelect','-game','-unattended',"-$flag","-abslog=`"$log`"")
if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
if($Review){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+=@('-nullrhi','-nosound')}
$started=Get-Date
$process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
if(-not $process.WaitForExit(600000)){Stop-Process -Id $process.Id;throw 'Frontend validation timed out'}
if($process.ExitCode -ne 0){throw "Frontend validation exited $($process.ExitCode); see $log"}
if($Review){
 $destination=Join-Path $reports "$prefix-${Width}x${Height}"
 New-Item -ItemType Directory -Force -Path $destination | Out-Null
 foreach($name in @('moon','mars','video','sound','gameplay','controls','controls150')){
  $file=Join-Path $projectRoot "Reports\Frontend20261028\frontend-$name.png"
  if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTime -lt $started){throw "Missing fresh screenshot: $file"}
  $bytes=[IO.File]::ReadAllBytes($file)
  if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $file"}
  $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
  if($w -ne $Width -or $h -ne $Height){throw "Unexpected image dimensions: ${w}x${h}"}
  Copy-Item -LiteralPath $file -Destination $destination
 }
 Write-Output "PASS: seven fresh screenshots at ${Width}x${Height}: $destination"
}else{
 $file=Join-Path $projectRoot 'Reports\frontend-checks.txt'
 if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTime -lt $started){throw 'Missing fresh frontend result'}
 $text=Get-Content -LiteralPath $file -Raw
 if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Frontend assertions failed: $file"}
 Copy-Item -LiteralPath $file -Destination (Join-Path $reports "$prefix-checks.txt")
 Write-Output $text
}
