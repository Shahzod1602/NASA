param(
 [switch]$Packaged,
 [switch]$Review,
 [switch]$Rendered,
 [ValidateSet('all','apollo','selene','mars')][string]$Mission='all',
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$projectRoot=if($Packaged){Join-Path $root 'Dist/Apollo12/Windows/LunarRescue'}else{$root}
$graphics=[bool]($Review -or $Rendered)
$editor=if($graphics){'UnrealEditor.exe'}else{'UnrealEditor-Cmd.exe'}
$exe=if($Packaged){Join-Path $projectRoot 'Binaries/Win64/LunarRescue.exe'}else{Join-Path 'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64' $editor}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing executable: $exe"}
$maps=@(
 [pscustomobject]@{Name='apollo';Map='/Game/Lunar/Maps/Apollo12';Prefix=''},
 [pscustomobject]@{Name='selene';Map='/Game/Lunar/Maps/MoonBase';Prefix=''},
 [pscustomobject]@{Name='mars';Map='/Game/Mars/Maps/MarsBase';Prefix='mars-'}
)
if($Mission -ne 'all'){$maps=@($maps|Where-Object Name -EQ $Mission)}
$kind=if($Review){'review'}elseif($Rendered){'rendered-test'}else{'test'}
$build=if($Packaged){'package'}else{'editor'}
$destination=Join-Path $root ("Reports/FieldOperations20261104/$build-$kind-${Width}x${Height}-"+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $destination|Out-Null
$configRoot=[IO.Path]::GetFullPath((Join-Path $projectRoot ('Saved/Config/'+$(if($Packaged){'Windows'}else{'WindowsEditor'}))))
$results=[Collections.Generic.List[object]]::new()
$exeHash=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
$modulePath=if($Packaged){$null}else{Join-Path $root 'Binaries/Win64/UnrealEditor-LunarRescue.dll'}
$moduleHash=if($modulePath){(Get-FileHash -LiteralPath $modulePath -Algorithm SHA256).Hash.ToLowerInvariant()}else{$null}

$sourceCommit=$null
if(Get-Command git -ErrorAction SilentlyContinue){try{$v=& git -C $root rev-parse HEAD 2>$null;if($LASTEXITCODE -eq 0 -and $v -match '^[0-9a-f]{40}$'){$sourceCommit=[string]$v}}catch{}}
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
 foreach($map in $maps){
  $caseResult=[ordered]@{case=$map.Name;map=$map.Map;flag=$(if($Review){'FieldOperationsReview'}else{'FieldOperationsTest'});outcome='failed';assertions=0;frames=0;config_restored=$false;quality_results=@()}
  $log=Join-Path $destination ($map.Name+'.log')
  $arguments=@($map.Map,'-game','-unattended','-nosound',('-'+$caseResult.flag),"-abslog=`"$log`"")
  if(-not $Packaged){$arguments=@("`"$root/LunarRescue.uproject`"")+$arguments}
  if($graphics){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
  $snapshot=ConfigSnapshot;$started=[DateTime]::UtcNow;$process=$null
  try{
   try{
    $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
    if(-not $process.WaitForExit(180000)){throw "Field operations $($map.Name) timed out; see $log"}
    if($process.ExitCode -ne 0){throw "Field operations $($map.Name) exited $($process.ExitCode); see $log"}
   }finally{
    try{if($process -and -not $process.HasExited){Stop-Process -Id $process.Id -Force -ErrorAction Stop;[void]$process.WaitForExit(5000)}}finally{RestoreConfig $snapshot;$caseResult.config_restored=$true}
   }
   FreshFile $log $started;$runLog=Get-Content -LiteralPath $log -Raw
   if($runLog -match 'Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|missing usage flag|FIELD_OPERATIONS_REVIEW FAILED'){throw "Crash/render failure: $log"}
   if($Review){
    if($runLog -notmatch 'FIELD_OPERATIONS_REVIEW COMPLETE'){throw "Missing successful review completion: $log"}
    $reviewFolder=Join-Path $projectRoot ("Reports/FieldOperations20261104/review-"+$map.Name)
    $report=Join-Path $reviewFolder 'field-operations-review-checks.txt';$caseResult.assertions=CheckReport $report $started
    $manifestPath=Join-Path $reviewFolder 'field-operations-manifest.json';FreshFile $manifestPath $started
    $manifest=Get-Content -LiteralPath $manifestPath -Raw|ConvertFrom-Json
    if($manifest.synthetic -isnot [bool] -or $manifest.synthetic -ne $true -or $manifest.human_playtest -isnot [bool] -or $manifest.human_playtest -ne $false -or $manifest.performance_measurement -isnot [bool] -or $manifest.performance_measurement -ne $false){throw 'Review must declare synthetic fixtures without human or performance claims'}
    $frames=@($manifest.frames);if($frames.Count -eq 0){throw 'Review has no staged frames'}
    $seen=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    $copyFolder=Join-Path $destination $map.Name;New-Item -ItemType Directory -Path $copyFolder|Out-Null
    $frameEvidence=[Collections.Generic.List[object]]::new()
    foreach($frame in $frames){
     $name=[string]$frame.name
     if($name -notmatch '^[a-z0-9]+(?:-[a-z0-9]+)*$' -or -not $seen.Add($name) -or $frame.layout_passed -isnot [bool] -or $frame.layout_passed -ne $true){throw "Invalid/duplicate frame name or failed arranged layout: $name"}
     $png=Join-Path $reviewFolder ($name+'.png');FreshFile $png $started
     $bytes=[IO.File]::ReadAllBytes($png);$signature=[byte[]]@(137,80,78,71,13,10,26,10)
     if($bytes.Length -lt 33){throw "Truncated PNG: $name"}
     for($i=0;$i -lt 8;$i++){if($bytes[$i] -ne $signature[$i]){throw "Invalid PNG signature: $name"}}
     if([Text.Encoding]::ASCII.GetString($bytes,12,4) -ne 'IHDR'){throw "Missing PNG IHDR: $name"}
     $w=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,16));$h=[Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($bytes,20))
     if($w -ne $Width -or $h -ne $Height){throw "Unexpected PNG ${w}x${h}: $name"}
     Copy-Item -LiteralPath $png -Destination $copyFolder
     $frameEvidence.Add([pscustomobject]@{name=$name;sha256=(Get-FileHash -LiteralPath $png -Algorithm SHA256).Hash.ToLowerInvariant();width=$w;height=$h;layout_passed=$true})
    }
    Copy-Item -LiteralPath $manifestPath -Destination $copyFolder;Copy-Item -LiteralPath $report -Destination $copyFolder
    $caseResult.frames=$frames.Count;$caseResult.frame_evidence=$frameEvidence
   }else{
    $report=Join-Path $projectRoot ("Reports/"+$map.Prefix+'field-operations-checks.txt');$caseResult.assertions=CheckReport $report $started
    Copy-Item -LiteralPath $report -Destination (Join-Path $destination ($map.Name+'-checks.txt'))
   }
   if((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant() -ne $exeHash){throw 'Executable changed during validation; cases cannot share one build receipt'}
   if($modulePath -and (Get-FileHash -LiteralPath $modulePath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $moduleHash){throw 'Editor game module changed during validation'}
   $caseResult.outcome='passed';$caseResult.started_utc=$started.ToString('o');$caseResult.completed_utc=[DateTime]::UtcNow.ToString('o');$results.Add([pscustomobject]$caseResult)
   Write-Output "PASS $($map.Name): $($caseResult.assertions) assertions, $($caseResult.frames) staged PNGs"
  }catch{$caseResult.error=$_.Exception.Message;$results.Add([pscustomobject]$caseResult);throw}
 }
 $after=SourceFingerprint;if($after.sha256 -ne $source.sha256){throw 'Source/data/config inputs changed during validation; rerun on a fixed snapshot'}
 $completed=$true
}finally{
 [ordered]@{schema=1;completed=$completed;cases=$results;executable_sha256=$exeHash;game_module_sha256=$moduleHash;source_commit=$sourceCommit;source_fingerprint=$source;packaged=[bool]$Packaged;review=[bool]$Review;rendered=$graphics;human_playtest=$false;performance_measurement=$false;network_isolation_tested=$false;width=$Width;height=$Height;completed_utc=[DateTime]::UtcNow.ToString('o')}|ConvertTo-Json -Depth 10|Set-Content -LiteralPath (Join-Path $destination 'run-summary.json') -Encoding utf8
}
Write-Output "Field operations task validation evidence: $destination"
