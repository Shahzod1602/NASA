param(
 [switch]$Packaged,
 [switch]$Rendered,
 [ValidateSet('all','stride','apollo')][string]$Suite='all',
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist/Apollo12/Windows/LunarRescue'}else{$root}
$editor=if($Rendered){'UnrealEditor.exe'}else{'UnrealEditor-Cmd.exe'}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries/Win64/LunarRescue.exe'}else{Join-Path 'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64' $editor}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$destination=Join-Path $root ('Reports/TaskFixes20261106/'+$(if($Packaged){'package'}else{'editor'})+'-'+$Suite+'-'+$(if($Rendered){'rendered'}else{'nullrhi'})+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $destination|Out-Null
$configRoot=[IO.Path]::GetFullPath((Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'}))))
$results=[Collections.Generic.List[object]]::new()
$exeHash=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
$modulePath=if($Packaged){$null}else{Join-Path $root 'Binaries/Win64/UnrealEditor-LunarRescue.dll'}
$moduleHash=if($modulePath){(Get-FileHash -LiteralPath $modulePath -Algorithm SHA256).Hash.ToLowerInvariant()}else{$null}
$sourceCommit=((& git -C $root rev-parse HEAD) -join '')
$cases=@()
if($Suite -in @('all','stride')){
 foreach($mission in @(@('apollo','/Game/Lunar/Maps/Apollo12',''),@('selene','/Game/Lunar/Maps/MoonBase',''),@('mars','/Game/Mars/Maps/MarsBase','mars-'))){$cases += [pscustomobject]@{name='stride-'+$mission[0];map=$mission[1];flag='StrideRegressionTest';report=$mission[2]+'stride-regression-checks.txt';limit=180;fixed_step=$false}}
}
if($Suite -in @('all','apollo')){$cases += [pscustomobject]@{name='apollo-interaction';map='/Game/Lunar/Maps/Apollo12';flag='ApolloInteractionRegressionTest';report='apollo-interaction-regression.txt';limit=1800;fixed_step=$true}}
function SourceFingerprint {
 $files=[Collections.Generic.List[object]]::new()
 foreach($folder in @('Source','Config','Content/Science/LunarWater')){
  $path=Join-Path $root $folder;if(-not(Test-Path -LiteralPath $path -PathType Container)){continue}
  foreach($file in Get-ChildItem -LiteralPath $path -File -Recurse|Where-Object{$_.Extension -in @('.cpp','.h','.cs','.ini','.json')}){
   $relative=$file.FullName.Substring($root.Length).TrimStart([char[]]'\/').Replace('\','/')
   $files.Add([pscustomobject]@{path=$relative;sha256=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()})
  }
 }
 $sorted=@($files|Sort-Object path);$canonical=($sorted|ForEach-Object{$_.path+':'+$_.sha256}) -join "`n"
 $sha=[Security.Cryptography.SHA256]::Create()
 try{$digest=[BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($canonical))).Replace('-','').ToLowerInvariant()}finally{$sha.Dispose()}
 return [pscustomobject]@{sha256=$digest;files=$sorted}
}
function AssertConfigTree {
 $ancestor=$configRoot
 while($ancestor){if(Test-Path -LiteralPath $ancestor){if((Get-Item -LiteralPath $ancestor).Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Config snapshot/restore refuses reparse-point ancestors'}};$next=Split-Path $ancestor -Parent;if($next -eq $ancestor){break};$ancestor=$next}
 if(Test-Path -LiteralPath $configRoot -PathType Container){foreach($d in Get-ChildItem -LiteralPath $configRoot -Directory -Recurse){if($d.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Config snapshot/restore refuses nested reparse points'}}}
}
function ConfigSnapshot {
 AssertConfigTree
 $state=[Collections.Generic.List[object]]::new()
 if(Test-Path -LiteralPath $configRoot -PathType Container){foreach($file in Get-ChildItem -LiteralPath $configRoot -File -Recurse -Filter '*.ini'){
  if($file.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Config snapshot refuses reparse-point files'}
  $state.Add([pscustomobject]@{path=$file.FullName;bytes=[IO.File]::ReadAllBytes($file.FullName);modified=$file.LastWriteTimeUtc})
 }}
 return ,$state
}
function RestoreConfig($Snapshot){
 AssertConfigTree
 $prefix=$configRoot.TrimEnd([char[]]'\/')+[IO.Path]::DirectorySeparatorChar
 $known=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
 foreach($entry in $Snapshot){[void]$known.Add($entry.path)}
 if(Test-Path -LiteralPath $configRoot -PathType Container){foreach($file in Get-ChildItem -LiteralPath $configRoot -File -Recurse -Filter '*.ini'){
  $resolved=[IO.Path]::GetFullPath($file.FullName)
  if(-not $resolved.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){throw 'Config restore escaped its fixed target folder'}
  if(-not $known.Contains($resolved)){Remove-Item -LiteralPath $resolved}
 }}
 foreach($entry in $Snapshot){
  $resolved=[IO.Path]::GetFullPath($entry.path)
  if(-not $resolved.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){throw 'Config snapshot escaped its fixed target folder'}
  New-Item -ItemType Directory -Path (Split-Path $resolved -Parent) -Force|Out-Null
  [IO.File]::WriteAllBytes($resolved,$entry.bytes);[IO.File]::SetLastWriteTimeUtc($resolved,$entry.modified)
  if([Convert]::ToBase64String([IO.File]::ReadAllBytes($resolved)) -cne [Convert]::ToBase64String($entry.bytes)){throw 'Config restore did not preserve exact original bytes'}
 }
}
function FreshFile([string]$Path,[DateTime]$Started){
 if(-not(Test-Path -LiteralPath $Path -PathType Leaf) -or (Get-Item -LiteralPath $Path).LastWriteTimeUtc -lt $Started){throw "Missing fresh artifact: $Path"}
}
function CheckReport([string]$Path,[DateTime]$Started){
 FreshFile $Path $Started;$text=Get-Content -LiteralPath $Path -Raw;$count=[regex]::Matches($text,'(?m)^PASS ').Count
 if($count -eq 0 -or $text -match '(?m)^FAIL ' -or [regex]::Matches($text,'(?m)^RESULT: 0 failures\r?$').Count -ne 1 -or [regex]::Matches($text,'(?m)^RESULT:').Count -ne 1){throw "Semantic check report failed: $Path"}
 return $count
}
$source=SourceFingerprint;$completed=$false
try{
 foreach($case in $cases){
  $entry=[ordered]@{name=$case.name;map=$case.map;flag=$case.flag;outcome='failed';fixed_step_simulation=$case.fixed_step;assertions=0}
  $log=Join-Path $destination ($case.name+'.log')
  $arguments=@($case.map,'-game','-unattended','-nosound',('-'+$case.flag),"-abslog=`"$log`"")
  if(-not $Packaged){$arguments=@("`"$root/LunarRescue.uproject`"")+$arguments}
  if($Rendered){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
  if($case.fixed_step){$arguments+=@('-benchmark','-fps=60','-ExecCmds="t.MaxFPS 0"')}
  $snapshot=ConfigSnapshot;$started=[DateTime]::UtcNow;$process=$null
  try{
   try{
    $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
    if(-not $process.WaitForExit($case.limit*1000)){throw "$($case.name) timed out; see $log"}
    if($process.ExitCode -ne 0){throw "$($case.name) exited $($process.ExitCode); see $log"}
   }finally{
    try{if($process -and -not $process.HasExited){Stop-Process -Id $process.Id -Force;[void]$process.WaitForExit(5000)}}finally{RestoreConfig $snapshot;$entry.config_restored=$true}
   }
   FreshFile $log $started;$runLog=Get-Content -LiteralPath $log -Raw
   if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material'){throw "Runtime failure: $log"}
   $report=Join-Path $projectRoot ('Reports/'+$case.report)
   $entry.assertions=CheckReport $report $started
   $copiedReport=Join-Path $destination ($case.name+'-checks.txt');Copy-Item -LiteralPath $report -Destination $copiedReport
   $entry.report_sha256=(Get-FileHash -LiteralPath $copiedReport -Algorithm SHA256).Hash.ToLowerInvariant()
   $entry.elapsed_wall_seconds=([DateTime]::UtcNow-$started).TotalSeconds
   $entry.arguments=$arguments
   if((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant() -ne $exeHash){throw 'Executable changed during validation'}
   if($modulePath -and (Get-FileHash -LiteralPath $modulePath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $moduleHash){throw 'Editor module changed during validation'}
   $entry.outcome='passed';$results.Add([pscustomobject]$entry)
   Write-Output "PASS $($case.name): $($entry.assertions) assertions"
  }catch{
   $entry.error=$_.Exception.Message
   $failureReport=Join-Path $projectRoot ('Reports/'+$case.report)
   if((Test-Path -LiteralPath $failureReport -PathType Leaf) -and (Get-Item -LiteralPath $failureReport).LastWriteTimeUtc -ge $started){
    $failureCopy=Join-Path $destination ($case.name+'-failed-checks.txt')
    Copy-Item -LiteralPath $failureReport -Destination $failureCopy
    $entry.failure_report_sha256=(Get-FileHash -LiteralPath $failureCopy -Algorithm SHA256).Hash.ToLowerInvariant()
   }
   $results.Add([pscustomobject]$entry);throw
  }
 }
 if((SourceFingerprint).sha256 -ne $source.sha256){throw 'Source/data/config inputs changed during validation'}
 $completed=$true
}finally{
 [ordered]@{schema=1;completed=$completed;cases=$results;executable_sha256=$exeHash;game_module_sha256=$moduleHash;source_commit=$sourceCommit;source_fingerprint=$source;packaged=[bool]$Packaged;rendered=[bool]$Rendered;human_playtest=$false;performance_measurement=$false;network_isolation_tested=$false;completed_utc=[DateTime]::UtcNow.ToString('o')}|ConvertTo-Json -Depth 10|Set-Content -LiteralPath (Join-Path $destination 'run-summary.json') -Encoding utf8
}
Write-Output "November task fix evidence: $destination"
