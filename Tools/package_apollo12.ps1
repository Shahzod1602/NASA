param([string]$EngineRoot='D:\UnrealEngine\UE_5.8')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force -Path "$root\Intermediate\ApolloTemp" | Out-Null
$env:TEMP="$root\Intermediate\ApolloTemp";$env:TMP=$env:TEMP
$saved=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Saved'
$preserved=Join-Path $root ('Backups\player-data-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
$playerFiles=@()
foreach($relative in @('SaveGames\Apollo12','Apollo12\Photos','Config\Windows\GameUserSettings.ini')){
 $folder=Join-Path $saved $relative
 if(Test-Path -LiteralPath $folder -PathType Leaf){$playerFiles+=Get-Item -LiteralPath $folder}
 elseif(Test-Path -LiteralPath $folder -PathType Container){$playerFiles+=Get-ChildItem -LiteralPath $folder -Recurse -File}
}
foreach($file in $playerFiles){
 $dest=Join-Path $preserved $file.FullName.Substring($saved.Length).TrimStart('\')
 New-Item -ItemType Directory -Force -Path (Split-Path $dest -Parent) | Out-Null
 Copy-Item -LiteralPath $file.FullName -Destination $dest
}
try{
 # Separate UBT processes keep the editor rules out of memory during the game link.
 foreach($target in @('LunarRescueEditor','LunarRescue')){
  & "$EngineRoot\Engine\Build\BatchFiles\Build.bat" $target Win64 Development "-Project=$root\LunarRescue.uproject" -WaitMutex -MaxParallelActions=1 -NoUBA -NoPCH -NoSharedPCH
  if($LASTEXITCODE -ne 0){throw "$target build failed: $LASTEXITCODE"}
 }
 & "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=$root\LunarRescue.uproject" -noP4 -platform=Win64 -clientconfig=Development -skipbuild -nocompileeditor -cook -map=/Game/Rescue/Maps/MissionSelect+/Game/Lunar/Maps/Apollo12+/Game/Lunar/Maps/MoonBase+/Game/Mars/Maps/MarsBase -stage -pak -archive "-archivedirectory=$root\Dist\Apollo12" -unattended -utf8output
 $packageCode=$LASTEXITCODE
}finally{
 # Restore files removed by staging; retain any newer copy already in the package.
 foreach($file in $playerFiles){
  if(-not(Test-Path -LiteralPath $file.FullName)){
   New-Item -ItemType Directory -Force -Path $file.DirectoryName | Out-Null
   Copy-Item -LiteralPath (Join-Path $preserved $file.FullName.Substring($saved.Length).TrimStart('\')) -Destination $file.FullName
  }
 }
 if($playerFiles.Count){Write-Output "Preserved $($playerFiles.Count) player files in $preserved"}
}
if($packageCode -ne 0){throw "Apollo 12 package failed: $packageCode"}
$sourceArt=Join-Path $root 'SourceArt';$credits=Join-Path $root 'Dist\Apollo12\Windows\Credits'
Get-ChildItem -LiteralPath $sourceArt -Recurse -File | Where-Object { $_.Name -like '*CREDITS*' -or $_.Name -like '*LICENSE*' } | ForEach-Object {
 $dest=Join-Path $credits $_.FullName.Substring($sourceArt.Length).TrimStart('\')
 New-Item -ItemType Directory -Force -Path (Split-Path $dest -Parent) | Out-Null
 Copy-Item -LiteralPath $_.FullName -Destination $dest
}
New-Item -ItemType Directory -Force -Path "$credits\Apollo12" | Out-Null
Copy-Item -LiteralPath "$root\SourceArt\Apollo12\manifest.json","$root\APOLLO12.md","$root\ONBOARDING_UPDATE.md","$root\CHECKPOINT_UPDATE.md","$root\SUIT_UPDATE.md","$root\ENGLISH_UPDATE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\POLISH_UPDATE.md","$root\HISTORICAL_AUDIT.md","$root\PLAYTEST_GUIDE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\SourceArt\Apollo12\Raw\NAC_DTM_APOLLO12_README.TXT","$root\SourceArt\Apollo12\Raw\NAC_DTM_APOLLO12.LBL" -Destination "$credits\Apollo12"
New-Item -ItemType Directory -Force -Path "$credits\Suit" | Out-Null
Copy-Item -LiteralPath "$root\SUIT_UPDATE.md" -Destination "$credits\Suit"
Copy-Item -LiteralPath "$root\PLAYING.txt" -Destination "$root\Dist\Apollo12\Windows\READ_ME.txt"
$runtime=Join-Path $EngineRoot 'Engine\Extras\Redist\en-us\vc_redist.x64.exe'
if(Test-Path -LiteralPath $runtime){
 $runtimeFolder=Join-Path $root 'Dist\Apollo12\Windows\Engine\Extras\Redist\en-us'
 New-Item -ItemType Directory -Force -Path $runtimeFolder | Out-Null
 Copy-Item -LiteralPath $runtime -Destination $runtimeFolder
}
Write-Output "Apollo 12 package: $root\Dist\Apollo12\Windows\LunarRescue.exe"
