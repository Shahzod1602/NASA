param([string]$EngineRoot='D:\UnrealEngine\UE_5.8',[switch]$StageOnly,[ValidateRange(256,2048)][int]$ManagedHeapMB=1024)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force -Path "$root\Intermediate\ApolloTemp" | Out-Null
$env:TEMP="$root\Intermediate\ApolloTemp";$env:TMP=$env:TEMP
$env:DOTNET_GCHeapHardLimit=('{0:X}' -f ($ManagedHeapMB*1MB));$env:DOTNET_GCConserveMemory='9'
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
 if(-not $StageOnly){
 # Separate UBT processes keep the editor rules out of memory during the game link.
 foreach($target in @('LunarRescueEditor','LunarRescue')){
  $buildFlags=@('-WaitMutex','-MaxParallelActions=1','-NoUBA','-NoPCH','-NoSharedPCH')
  # The monolithic Windows symbol link exceeds this machine's commit budget.
  # Keep editor symbols for development; the distributed game omits symbols.
  if($target -eq 'LunarRescue'){$buildFlags+=@('-NoDebugInfo','-NoLinkerDebugInfo')}
  & "$EngineRoot\Engine\Build\BatchFiles\Build.bat" $target Win64 Development "-Project=$root\LunarRescue.uproject" @buildFlags
  if($LASTEXITCODE -ne 0){throw "$target build failed: $LASTEXITCODE"}
 }
 # Verify persisted shared materials after importing/reducing skeletal assets.
 $bindingLog=Join-Path $root 'Reports\package-realism-bindings.log'
 $bindingArgs=@("`"$root\LunarRescue.uproject`"",'-run=pythonscript',"-script=`"$root\Tools\bind_realism_materials.py`"",'-unattended','-nop4','-nosound','-nullrhi',"-abslog=`"$bindingLog`"")
 $binding=Start-Process -FilePath "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" -ArgumentList $bindingArgs -WindowStyle Hidden -PassThru
 if(-not $binding.WaitForExit(120000)){Stop-Process -Id $binding.Id;throw 'Suit material binding timed out'}
 if($binding.ExitCode -ne 0 -or (Get-Content -LiteralPath $bindingLog -Raw) -notmatch 'REALISM_SUIT_BINDINGS_VERIFIED'){throw 'Suit material binding failed'}
 }
 elseif(-not(Test-Path -LiteralPath "$root\Binaries\Win64\LunarRescue.exe") -or -not(Test-Path -LiteralPath "$root\Saved\Cooked\Windows\ue.projectstore")){throw 'StageOnly requires an existing compiled game and completed Windows cook'}
 $cookFlag=if($StageOnly){'-skipcook'}else{'-cook'}
 # Bound IoStore reads/compression instead of reserving gigabytes in parallel.
 & "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=$root\LunarRescue.uproject" -noP4 -platform=Win64 -clientconfig=Development -skipbuild -nocompileeditor $cookFlag -map=/Game/Rescue/Maps/MissionSelect+/Game/Lunar/Maps/Apollo12+/Game/Lunar/Maps/MoonBase+/Game/Mars/Maps/MarsBase -stage -pak -archive "-archivedirectory=$root\Dist\Apollo12" -unattended -utf8output '-AdditionalIoStoreOptions="-MaxConcurrentSourceReads=1 -MaxSourceBufferMemory=536870912 -MaxCompressionBufferMemory=268435456"'
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
Copy-Item -LiteralPath "$root\REALISM_UPDATE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\CLARITY_UPDATE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\LOADING_UPDATE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\EXPEDITION_POLISH.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\EXPEDITION_EXPERIENCE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\Tools\fixtures\expedition-playtest-form.md" -Destination "$credits\Apollo12\PLAYTEST_FORM.md"
Copy-Item -LiteralPath "$root\FIELDWORK_UPDATE.md" -Destination "$credits\Apollo12"
Copy-Item -LiteralPath "$root\SourceArt\Apollo12\Raw\NAC_DTM_APOLLO12_README.TXT","$root\SourceArt\Apollo12\Raw\NAC_DTM_APOLLO12.LBL" -Destination "$credits\Apollo12"
New-Item -ItemType Directory -Force -Path "$credits\Suit" | Out-Null
Copy-Item -LiteralPath "$root\SUIT_UPDATE.md" -Destination "$credits\Suit"
Copy-Item -LiteralPath "$root\SUIT_REFINEMENT.md" -Destination "$credits\Suit"
Copy-Item -LiteralPath "$root\PLAYING.txt" -Destination "$root\Dist\Apollo12\Windows\READ_ME.txt"
$runtime=Join-Path $EngineRoot 'Engine\Extras\Redist\en-us\vc_redist.x64.exe'
if(Test-Path -LiteralPath $runtime){
 $runtimeFolder=Join-Path $root 'Dist\Apollo12\Windows\Engine\Extras\Redist\en-us'
 New-Item -ItemType Directory -Force -Path $runtimeFolder | Out-Null
 Copy-Item -LiteralPath $runtime -Destination $runtimeFolder
}
# A later build must not inherit this iteration's manual verification claim.
$stagedGame=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Binaries\Win64\LunarRescue.exe'
$engineVersion=Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine\Build\Build.version') -Raw | ConvertFrom-Json
$localBuild=[ordered]@{
 schema_version=1
 build_kind='local-development'
 published_release=$false
 base_git_commit=((& git -C $root rev-parse HEAD) -join '')
 source_worktree_dirty=(@(& git -C $root status --porcelain).Count -gt 0)
 engine_version="$($engineVersion.MajorVersion).$($engineVersion.MinorVersion).$($engineVersion.PatchVersion)"
 generated_utc=[DateTime]::UtcNow.ToString('o')
 executable='LunarRescue/Binaries/Win64/LunarRescue.exe'
 executable_sha256=(Get-FileHash -LiteralPath $stagedGame -Algorithm SHA256).Hash.ToLowerInvariant()
 verification_status='Not certified by packaging; run verification for this build and its assets.'
}
$localBuild | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $root 'Dist\Apollo12\Windows\BUILD_INFO.json') -Encoding UTF8
Write-Output "Apollo 12 package: $root\Dist\Apollo12\Windows\LunarRescue.exe"
