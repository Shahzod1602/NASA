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
$kind=if($Review){'review'}else{'checks'}
$prefix=if($Packaged){'package'}else{'editor'}
$reports=Join-Path $root 'Reports\LoadingTypography20261102'
$destination=Join-Path $reports "$prefix-$kind-${Width}x${Height}-$(Get-Date -Format 'yyyyMMdd-HHmmss-fff')"
New-Item -ItemType Directory -Path $destination -Force | Out-Null
$log=Join-Path $destination 'run.log'
$flag=if($Review){'-LoadingTypographyReview'}else{'-LoadingTypographyTest'}
$arguments=@('/Game/Rescue/Maps/MissionSelect','-game','-unattended','-nosound',$flag,"-abslog=`"$log`"")
if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
if($Review){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
$config=Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'})+'/GameUserSettings.ini')
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[IO.File]::ReadAllBytes($config)}else{$null}
$started=[DateTime]::UtcNow
try{
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(180000)){
  if(-not $process.HasExited){Stop-Process -Id $process.Id -ErrorAction SilentlyContinue}
  $process.WaitForExit()
  throw "Loading typography timed out; see $log"
 }
 if($process.ExitCode -ne 0){throw "Loading typography exited $($process.ExitCode); see $log"}
}finally{
 if($hadConfig){New-Item -ItemType Directory -Force -Path (Split-Path $config -Parent) | Out-Null;[IO.File]::WriteAllBytes($config,$configBytes)}
 elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
$runLog=Get-Content -LiteralPath $log -Raw
if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag|LOADING_TYPOGRAPHY_REVIEW FAILED'){throw "Loading typography run failed; see $log"}
if($Review){
 $source=Join-Path $projectRoot 'Reports\LoadingTypography20261102\staged'
 $manifestFile=Join-Path $source 'layout-manifest.json'
 if(-not(Test-Path -LiteralPath $manifestFile) -or (Get-Item -LiteralPath $manifestFile).LastWriteTimeUtc -lt $started){throw 'Missing fresh loading layout manifest'}
 $manifest=Get-Content -LiteralPath $manifestFile -Raw | ConvertFrom-Json
 if(-not $manifest.synthetic -or $manifest.human_playtest -or $manifest.performance_measurement -or $manifest.actual_travel -or $runLog -notmatch 'LOADING_TYPOGRAPHY_REVIEW COMPLETE'){throw 'Layout snapshots must be labelled as staged visuals rather than actual travel or performance evidence'}
 $names=@('apollo','apollo150','selene','selene150','mars','mars150','menu','menu150','apollo150-zero','apollo150-ready')
 if(@($manifest.frames).Count -ne $names.Count){throw 'Unexpected loading layout frame count'}
 foreach($name in $names){
  $frame=@($manifest.frames | Where-Object name -EQ $name)
  if($frame.Count -ne 1 -or -not $frame[0].slate_measured){throw "Missing measured layout fixture: $name"}
  $scale=if($name -like '*150*'){1.5}else{1.0};$percent=if($name -eq 'apollo150-zero'){0}elseif($name -eq 'apollo150-ready'){100}else{85}
  if($frame[0].text_scale -ne $scale -or $frame[0].percent -ne $percent){throw "Incorrect text scale/progress fixture: $name"}
  $file=Join-Path $source "$name.png"
  if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTimeUtc -lt $started){throw "Missing fresh PNG: $name"}
  $bytes=[IO.File]::ReadAllBytes($file)
  if($bytes.Length -lt 24 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $name"}
  $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
  if($w -ne $Width -or $h -ne $Height){throw "Incorrect PNG dimensions ${w}x${h}: $name"}
  Copy-Item -LiteralPath $file -Destination $destination
 }
 foreach($base in @('apollo','selene','mars','menu')){
  $normal=$manifest.frames | Where-Object name -EQ $base;$large=$manifest.frames | Where-Object name -EQ ($base+'150')
  if($normal.footer_top -le 488 -or $large.footer_top -ge $normal.footer_top){throw "Typography did not give default art more room and expand for large text: $base"}
 }
 Copy-Item -LiteralPath $manifestFile -Destination $destination
 $summary=[ordered]@{kind='staged-layout-review';frames=10;human_playtest=$false;actual_travel=$false;performance_measurement=$false;resolution="${Width}x${Height}"}
 Write-Output 'PASS: ten fresh loading layout snapshots; actual progress/world hold requires verify_loading.ps1 -Graphics.'
}else{
 $result=Join-Path $projectRoot 'Reports\loading-typography-checks.txt'
 if(-not(Test-Path -LiteralPath $result) -or (Get-Item -LiteralPath $result).LastWriteTimeUtc -lt $started){throw 'Missing fresh loading typography checks'}
 $text=Get-Content -LiteralPath $result -Raw
 $passes=[regex]::Matches($text,'(?m)^PASS ').Count
 if(-not $passes -or $text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw "Loading typography assertions failed; see $result"}
 Copy-Item -LiteralPath $result -Destination $destination
 $summary=[ordered]@{kind='layout-checks';passed=$passes;failures=0}
 Write-Output $text
}
$summary.executable_sha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
$summary.completed_utc=[DateTime]::UtcNow.ToString('o')
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $destination 'run-summary.json') -Encoding utf8
Write-Output "Loading typography evidence: $destination"
