[CmdletBinding(DefaultParameterSetName='Archive')]
param(
 [Parameter(Mandatory=$true)][ValidatePattern('^P0[1-8]$')][string]$Participant,
 [Parameter(Mandatory=$true,ParameterSetName='Archive')][string]$Archive,
 [Parameter(Mandatory=$true,ParameterSetName='Package')][string]$PackageRoot,
 [switch]$PrepareOnly
)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Split-Path $PSScriptRoot -Parent))
$destination=[IO.Path]::GetFullPath((Join-Path $root "Dist\Playtests\$Participant"))
if(Test-Path -LiteralPath $destination){throw 'This participant already has a test installation. Use it to resume; do not overwrite the existing evidence.'}

function Assert-NoReparseAncestors([string]$Path){
 $directory=[IO.DirectoryInfo]::new($Path)
 while($null -ne $directory){
  if($directory.Exists -and ($directory.Attributes -band [IO.FileAttributes]::ReparsePoint)){
   throw "A symbolic link/junction is not allowed in the installation path: $($directory.FullName)"
  }
  $directory=$directory.Parent
 }
}
function Test-ExcludedPlaytestPath([string]$Relative,[bool]$IsDirectory=$false){
 $parts=$Relative.Replace('\','/').Split('/')
 if(@($parts | Where-Object {$_ -ieq 'Saved' -or $_ -ieq 'Reports'}).Count){return $true}
 if($IsDirectory){return $false}
 $name=$parts[-1]
 return $name -ilike 'Manifest_*' -or [IO.Path]::GetExtension($name) -iin @('.pdb','.log','.csv') -or $Relative -ieq 'preparation.json'
}
function Find-PlaytestBootstrap($Files){
 $names=@($Files | ForEach-Object {$_.Relative})
 $bootstraps=@($names | Where-Object {$_ -imatch '(^|/)LunarRescue\.exe$' -and $_ -inotmatch '(^|/)Binaries/'})
 if($bootstraps.Count -ne 1){throw 'Expected exactly one game bootstrap in the source installation'}
 $prefix=$bootstraps[0].Substring(0,$bootstraps[0].Length-'LunarRescue.exe'.Length)
 $required=@('LunarRescue.exe','LunarRescue/Binaries/Win64/LunarRescue.exe','LunarRescue/Content/Paks/LunarRescue-Windows.pak','LunarRescue/Content/Paks/LunarRescue-Windows.utoc','LunarRescue/Content/Paks/LunarRescue-Windows.ucas')
 foreach($relative in $required){
  $match=@($Files | Where-Object {$_.Relative -ieq ($prefix+$relative)})
  if($match.Count -ne 1 -or $match[0].Length -le 0){throw "Missing or empty required runtime file: $prefix$relative"}
 }
 if(-not @($names | Where-Object {$_.StartsWith($prefix+'Credits/',[StringComparison]::OrdinalIgnoreCase)}).Count){throw 'Missing required Credits files'}
 return $bootstraps[0]
}
Assert-NoReparseAncestors $destination
$files=[Collections.Generic.List[object]]::new()
$seen=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$zipHandle=$null
try {
 if($PSCmdlet.ParameterSetName -eq 'Package'){
  $source=[IO.Path]::GetFullPath((Resolve-Path -LiteralPath $PackageRoot).ProviderPath)
  if(-not(Test-Path -LiteralPath $source -PathType Container)){throw 'PackageRoot must be a runtime directory'}
  Assert-NoReparseAncestors $source
  $sourcePrefix=$source.TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar
  $targetPrefix=$destination.TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar
  if($destination.Equals($source,[StringComparison]::OrdinalIgnoreCase) -or $destination.StartsWith($sourcePrefix,[StringComparison]::OrdinalIgnoreCase) -or $source.StartsWith($targetPrefix,[StringComparison]::OrdinalIgnoreCase)){
   throw 'Source and participant destination must be separate, non-nested directories'
  }
  # Walk explicitly: never descend into excluded data or a reparse point.
  $pending=[Collections.Generic.Stack[string]]::new();$pending.Push($source)
  while($pending.Count){
   foreach($item in Get-ChildItem -LiteralPath $pending.Pop() -Force){
    $relative=$item.FullName.Substring($sourcePrefix.Length).Replace('\','/')
    if(Test-ExcludedPlaytestPath $relative $item.PSIsContainer){continue}
    if($item.Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Cannot copy a symbolic link/junction: $relative"}
    if($item.PSIsContainer){$pending.Push($item.FullName);continue}
    if(-not $seen.Add($relative)){throw "Duplicate runtime path: $relative"}
    $files.Add([pscustomobject]@{Relative=$relative;Source=$item.FullName;Entry=$null;Length=$item.Length})
   }
  }
 }else{
  $source=[IO.Path]::GetFullPath((Resolve-Path -LiteralPath $Archive).ProviderPath)
  if(-not(Test-Path -LiteralPath $source -PathType Leaf)){throw 'Archive must be a ZIP file'}
  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $zipHandle=[IO.Compression.ZipFile]::OpenRead($source)
  foreach($entry in $zipHandle.Entries){
   $relative=$entry.FullName.Replace('\','/')
   if([string]::IsNullOrEmpty($relative)){continue}
   if($relative.StartsWith('/') -or $relative.Contains(':') -or @($relative.Split('/') | Where-Object {$_ -eq '..' -or $_ -eq '.'}).Count){throw "Unsafe ZIP path: $relative"}
   if(($entry.ExternalAttributes -shr 16 -band 0xF000) -eq 0xA000){throw "ZIP symbolic links are not supported: $relative"}
   if($relative.EndsWith('/') -or (Test-ExcludedPlaytestPath $relative)){continue}
   if(-not $seen.Add($relative)){throw "Duplicate ZIP path: $relative"}
   $files.Add([pscustomobject]@{Relative=$relative;Source=$null;Entry=$entry;Length=$entry.Length})
  }
 }
 # Validate everything before creating a participant installation.
 $bootstrapRelative=Find-PlaytestBootstrap $files
 if($PSCmdlet.ParameterSetName -eq 'Package' -and $bootstrapRelative -ine 'LunarRescue.exe'){throw 'PackageRoot must directly contain LunarRescue.exe (for example Dist/Apollo12/Windows)'}
 $plannedTargets=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
 foreach($row in $files){
  $target=[IO.Path]::GetFullPath((Join-Path $destination $row.Relative))
  if(-not $plannedTargets.Add($target)){throw "Duplicate canonical runtime target: $($row.Relative)"}
  if(-not $target.StartsWith($destination.TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw "Runtime path escapes the participant directory: $($row.Relative)"}
  $parts=$row.Relative.Split('/')
  for($i=1;$i -lt $parts.Count;$i++){if($seen.Contains(($parts[0..($i-1)] -join '/'))){throw "A runtime file is also used as a directory: $($row.Relative)"}}
 }
 $baseCommit=$null;$sourceDirty=$null
 if($PSCmdlet.ParameterSetName -eq 'Package' -and (Get-Command git -ErrorAction SilentlyContinue)){
  $revision=& git --no-optional-locks -C $root rev-parse HEAD 2>$null
  if($LASTEXITCODE -eq 0){$baseCommit=($revision -join '').Trim()}
  $changes=& git --no-optional-locks -C $root status --porcelain 2>$null
  if($LASTEXITCODE -eq 0){$sourceDirty=(@($changes).Count -gt 0)}
 }
 # No Force and CreateNew file streams: an existing participant is never replaced.
 New-Item -ItemType Directory -Path $destination | Out-Null
 foreach($row in $files){
  $target=[IO.Path]::GetFullPath((Join-Path $destination $row.Relative))
  [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target)) | Out-Null
  $inputStream=if($row.Entry){$row.Entry.Open()}else{[IO.File]::OpenRead($row.Source)}
  try{
   $outputStream=[IO.File]::Open($target,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write,[IO.FileShare]::None)
   try{$inputStream.CopyTo($outputStream)}finally{$outputStream.Dispose()}
  }finally{$inputStream.Dispose()}
 }
 $info=[ordered]@{
  participant=$Participant
  prepared_utc=[DateTime]::UtcNow.ToString('o')
  source_kind=if($PSCmdlet.ParameterSetName -eq 'Package'){'local-package'}else{'archive'}
  source_path=$source
  source_git_base_commit=$baseCommit
  source_git_worktree_dirty=$sourceDirty
  committed_release_claim=$false
  source_build_info_preserved=(@($files | Where-Object {[IO.Path]::GetFileName($_.Relative) -ieq 'BUILD_INFO.json'}).Count -gt 0)
  source_build_info_verified=$false
  copied_files=$files.Count
  exclusions=@('Saved and Reports subtrees','PDB/log/CSV files','Manifest_* files','prior root preparation.json')
  prepared_without_launch=[bool]$PrepareOnly
 }
 $info | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $destination 'preparation.json') -Encoding UTF8
}finally{
 if($zipHandle){$zipHandle.Dispose()}
}
$bootstrap=Join-Path $destination $bootstrapRelative
Write-Output "Anonymous participant $Participant; isolated installation: $destination"
Write-Output "Prepared $($files.Count) clean runtime files. Source BUILD_INFO, if present, is preserved without certifying it as a release."
if($PrepareOnly){
 Write-Output "Prepared only; no game launched. Start manually when the participant is ready: $bootstrap"
}else{
 Start-Process -FilePath $bootstrap -WorkingDirectory (Split-Path $bootstrap -Parent) -WindowStyle Normal
}
