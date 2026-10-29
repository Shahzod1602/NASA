param(
 [switch]$Packaged,
 [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='current',
 [ValidateRange(640,1920)][int]$Width=960,
 [ValidateRange(360,1080)][int]$Height=540
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'}else{$root}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries\Win64\LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$folder=Join-Path $root ("Reports\ExpeditionExperience\motion-$Label-"+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $folder -Force | Out-Null
$log=Join-Path $folder 'capture.log'
$argsList=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-ExpeditionMotionReview','-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height",'-nosound','-benchmark','-FPS=60',"-abslog=`"$log`"")
if(-not $Packaged){$argsList=@("`"$root\LunarRescue.uproject`"")+$argsList}
$config=Join-Path $projectRoot ('Saved\Config\'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'\GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$started=Get-Date
try{
 $process=Start-Process -FilePath $exe -ArgumentList $argsList -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(180000)){Stop-Process -Id $process.Id;$process.WaitForExit();throw "Motion review timed out: $log"}
 if($process.ExitCode -ne 0){throw "Motion review failed ($($process.ExitCode)): $log"}
}finally{
 if($hadConfig){New-Item -ItemType Directory -Path (Split-Path $config -Parent) -Force | Out-Null;[IO.File]::WriteAllBytes($config,$configBytes)}
 elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
$logText=Get-Content -LiteralPath $log -Raw
if($logText -notmatch 'EXPEDITION_MOTION_REVIEW 120 requested frames' -or $logText -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material'){throw "Motion review did not complete cleanly: $log"}
$source=Join-Path $projectRoot 'Reports\ExpeditionMotion'
for($i=0;$i -lt 120;$i++){
 $name='motion-{0:000}.png' -f $i
 $file=Join-Path $source $name
 if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTime -lt $started){throw "Missing fresh frame $name"}
 $bytes=[IO.File]::ReadAllBytes($file)
 if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid motion PNG: $name"}
 $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
 if($w -ne $Width -or $h -ne $Height){throw "Wrong motion dimensions: ${w}x${h}"}
 Copy-Item -LiteralPath $file -Destination $folder
}
foreach($name in @('motion.csv','manifest.json')){
 $file=Join-Path $source $name
 if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTime -lt $started){throw "Missing motion metadata $name"}
 Copy-Item -LiteralPath $file -Destination $folder
}
$manifest=Get-Content -LiteralPath (Join-Path $folder 'manifest.json') -Raw | ConvertFrom-Json
$rows=@(Import-Csv -LiteralPath (Join-Path $folder 'motion.csv'))
if(-not $manifest.synthetic -or $manifest.requested_frames -ne 120 -or $rows.Count -ne 120){throw 'Invalid synthetic sequence declaration'}
Write-Output "PASS: 120 fresh synthetic motion frames and telemetry: $folder"
Write-Output 'Encode optionally: ffmpeg -framerate 12 -i motion-%03d.png -c:v libx264 -pix_fmt yuv420p motion.mp4'
