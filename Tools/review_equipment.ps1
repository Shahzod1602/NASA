param(
 [ValidateSet('All','Apollo','Selene','Mars')][string]$Mission='All',
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720,
 [string]$Output,
 [string[]]$Shots=@()
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$editor='D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$script=Join-Path $PSScriptRoot 'review_equipment.py'
if(-not (Test-Path -LiteralPath $editor -PathType Leaf)){throw "Missing editor: $editor"}
if(-not (Test-Path -LiteralPath $script -PathType Leaf)){throw "Missing review script: $script"}
if(-not $Output){$Output=Join-Path $root ('Reports/Feedback20261102/equipment-python-'+$Width+'x'+$Height+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))}
$Output=[IO.Path]::GetFullPath($Output)
$reportsPrefix=[IO.Path]::GetFullPath((Join-Path $root 'Reports')).TrimEnd('\')+'\'
if(-not $Output.StartsWith($reportsPrefix,[StringComparison]::OrdinalIgnoreCase)){throw 'Output must be a fresh child directory of this project Reports'}
if(Test-Path -LiteralPath $Output){throw "Use a fresh review folder: $Output"}
New-Item -ItemType Directory -Path $Output -Force | Out-Null
$cases=@(
 [pscustomobject]@{Name='apollo';Select='Apollo';Map='/Game/Lunar/Maps/Apollo12';MapName='Apollo12';Frames=8;Names=@('surveyor-overview','surveyor-camera','surveyor-scoop','surveyor-cable','alsep-seismometer','alsep-central','intrepid-wide','intrepid-side-handles')},
 [pscustomobject]@{Name='selene';Select='Selene';Map='/Game/Lunar/Maps/MoonBase';MapName='MoonBase';Frames=5;Names=@('selene-power-socket','selene-hdu-contact','selene-rover-wheel-axes','selene-solar-underside','selene-nomad-reference')},
 [pscustomobject]@{Name='mars';Select='Mars';Map='/Game/Mars/Maps/MarsBase';MapName='MarsBase';Frames=2;Names=@('ares-outpost','ares-mesa-talus')}
) | Where-Object {$Mission -eq 'All' -or $_.Select -eq $Mission}
$availableNames=@($cases | ForEach-Object {$_.Names})
if($Shots.Count){
 if(@($Shots | Sort-Object -Unique).Count -ne $Shots.Count){throw 'Duplicate -Shots names are not allowed'}
 foreach($shot in $Shots){if($availableNames -cnotcontains $shot){throw "Unknown exact shot name or mission mismatch: $shot"}}
 $cases=@($cases | ForEach-Object {$case=$_;$case.Names=@($case.Names | Where-Object {$Shots -ccontains $_});$case.Frames=$case.Names.Count;if($case.Frames){$case}})
}
$results=[Collections.Generic.List[object]]::new()
foreach($case in $cases){
 $folder=Join-Path $Output $case.Name;$log=Join-Path $Output ($case.Name+'.log')
 $forwardScript=$script.Replace('\','/');$forwardFolder=$folder.Replace('\','/')
 $argsList=@(('"'+(Join-Path $root 'LunarRescue.uproject')+'"'),$case.Map,'-game','-EquipmentPythonReview','-unattended','-nosound','-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height",('-EquipmentReviewOutput="'+$forwardFolder+'"'),"-EquipmentReviewWidth=$Width","-EquipmentReviewHeight=$Height",('-ExecCmds="py '+$forwardScript+'"'),('-abslog="'+$log+'"'))
 if($Shots.Count){$argsList+=('-EquipmentReviewShots='+($case.Names -join ','))}
 $configs=@('WindowsEditor','Windows') | ForEach-Object {$file=Join-Path $root ('Saved/Config/'+$_+'/GameUserSettings.ini');$exists=Test-Path -LiteralPath $file;[pscustomobject]@{Path=$file;Exists=$exists;Bytes=$(if($exists){[IO.File]::ReadAllBytes($file)}else{$null})}}
 $started=[DateTime]::UtcNow
 try{
  $process=Start-Process -FilePath $editor -ArgumentList $argsList -WindowStyle Hidden -PassThru
  if(-not $process.WaitForExit(240000)){if(-not $process.HasExited){Stop-Process -Id $process.Id};$process.WaitForExit();throw "Equipment $($case.Name) timed out; see $log"}
  if($process.ExitCode -ne 0){throw "Equipment $($case.Name) exited $($process.ExitCode); see $log"}
 }finally{
  foreach($config in $configs){if($config.Exists){New-Item -ItemType Directory -Path (Split-Path $config.Path -Parent) -Force | Out-Null;[IO.File]::WriteAllBytes($config.Path,$config.Bytes)}elseif(Test-Path -LiteralPath $config.Path){Remove-Item -LiteralPath $config.Path}}
 }
 $manifestPath=Join-Path $folder 'equipment-manifest.json'
 if(-not (Test-Path -LiteralPath $manifestPath) -or (Get-Item -LiteralPath $manifestPath).LastWriteTimeUtc -lt $started){throw "Missing fresh equipment manifest: $manifestPath"}
 $manifest=Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
 $runLog=Get-Content -LiteralPath $log -Raw
 if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag|EQUIPMENT_PYTHON_REVIEW FAILED'){throw "Engine/Python/material failure: $log"}
 if(-not $manifest.success -or -not $manifest.synthetic -or $manifest.human_playtest -or $manifest.performance_measurement){throw "Invalid review scope or failure: $manifestPath"}
 if($runLog -notmatch 'EQUIPMENT_PYTHON_REVIEW COMPLETE'){throw "No completed capture marker: $log"}
 if($manifest.map -ne $case.MapName -or @($manifest.dimensions).Count -ne 2 -or $manifest.dimensions[0] -ne $Width -or $manifest.dimensions[1] -ne $Height){throw "Wrong map or dimensions: $manifestPath"}
 if(@($manifest.frames).Count -ne $case.Frames){throw "Wrong frame count: $manifestPath"}
 if((@($manifest.frames.name) -join '|') -cne ($case.Names -join '|')){throw "Wrong or duplicate equipment shot plan: $manifestPath"}
 if(@($manifest.frames.file | Sort-Object -Unique).Count -ne $case.Frames){throw "Duplicate screenshot file: $manifestPath"}
 foreach($frame in $manifest.frames){
  if($frame.file -cne ($frame.name+'.png')){throw 'Screenshot filename does not match its planned view'}
  if($frame.file -notmatch '^[a-z0-9][a-z0-9-]*\.png$'){throw 'Unexpected screenshot filename'}
  $file=Join-Path $folder $frame.file
  if(-not(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).LastWriteTimeUtc -lt $started){throw "Missing fresh screenshot: $file"}
  $bytes=[IO.File]::ReadAllBytes($file)
  if($bytes.Length -lt 1024 -or [Text.Encoding]::ASCII.GetString($bytes,1,3) -ne 'PNG'){throw "Invalid PNG: $file"}
  $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
  if($w -ne $Width -or $h -ne $Height -or (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant() -ne $frame.sha256){throw "Dimensions/hash mismatch: $file"}
 }
 $results.Add([pscustomobject]@{Mission=$case.Name;Frames=$case.Frames;Manifest=$manifestPath;Outcome='passed';HumanPlaytest=$false;PerformanceMeasurement=$false})
 Write-Output "PASS equipment $($case.Name): $($case.Frames) fresh ${Width}x${Height} staged PNGs"
}
[pscustomobject]@{CompletedUtc=[DateTime]::UtcNow.ToString('o');Executable=$editor;ScriptSha256=(Get-FileHash -LiteralPath $script -Algorithm SHA256).Hash.ToLowerInvariant();Synthetic=$true;RequestedShots=$Shots;HumanPlaytest=$false;PerformanceMeasurement=$false;Cases=$results} | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $Output 'equipment-run-summary.json') -Encoding utf8
Write-Output "Equipment review evidence: $Output"
