param([switch]$Packaged)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$project=if($Packaged){Join-Path $root 'Dist/Apollo12/Windows/LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $project 'Binaries/Win64/LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$reports=Join-Path $root 'Reports/ExpeditionExperience20261029'
New-Item -ItemType Directory -Path $reports -Force | Out-Null
$prefix=if($Packaged){'package'}else{'editor'}
$config=Join-Path $project ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
try{
 foreach($case in @(@('LunarMovementTest','movement-checks.txt','mars-movement'),@('LunarLedgeTest','ledge-checks.txt','mars-ledge'))){
  $log=Join-Path $reports ("final-$prefix-$($case[2]).log")
  $arguments=@('/Game/Mars/Maps/MarsBase','-game','-unattended',"-$($case[0])",'-nullrhi','-nosound',"-abslog=`"$log`"")
  if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
  $started=Get-Date
  $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
  if(-not $process.WaitForExit(180000)){Stop-Process -Id $process.Id;$process.WaitForExit();throw "$($case[2]) timed out"}
  if($process.ExitCode -ne 0){throw "$($case[2]) exited $($process.ExitCode); see $log"}
  $file=Join-Path $project ('Reports/mars-'+$case[1])
  if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTime -lt $started){throw "Missing fresh result: $file"}
  $result=Get-Content -LiteralPath $file -Raw
  if($result -match '(?m)^FAIL ' -or $result -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Mars movement assertion failed: $file"}
  Copy-Item -LiteralPath $file -Destination (Join-Path $reports ("final-$prefix-$($case[2]).txt")) -Force
  Write-Output $result
 }
}finally{
 if($hadConfig){New-Item -ItemType Directory -Path (Split-Path $config -Parent) -Force | Out-Null;[IO.File]::WriteAllBytes($config,$configBytes)}
 elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
