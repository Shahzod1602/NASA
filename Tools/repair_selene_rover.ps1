param([switch]$Apply)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Split-Path $PSScriptRoot -Parent)).TrimEnd('\')
$map=Join-Path $root 'Content/Lunar/Maps/MoonBase.umap'
$editor='D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$python=Join-Path $PSScriptRoot 'repair_selene_rover.py'
$fixture=Join-Path $PSScriptRoot 'fixtures/selene-rover-wheel-contacts.json'
foreach($path in @($map,$editor,$python,$fixture)){if(-not(Test-Path -LiteralPath $path -PathType Leaf)){throw "Missing required file: $path"}}
if((Get-Item -LiteralPath $map).Length -lt 1024){throw 'Expected real map binary, not a small LFS pointer'}
if(-not $Apply){Write-Output 'Prepared repair only. Use -Apply after all engine jobs exit. Scope: existing MoonBase Lunokhod pitch15->pitch0/yaw15 plus terrain-tested shallowZ; no other actor or map changes.';return}
if(Get-Process UnrealEditor,UnrealEditor-Cmd,LunarRescue -ErrorAction SilentlyContinue){throw 'Close/wait for existing engine jobs before this exclusive Editor repair'}
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$backup=[IO.Path]::GetFullPath((Join-Path $root ('Backups/selene-rover-'+$stamp)))
$output=[IO.Path]::GetFullPath((Join-Path $root ('Reports/Feedback20261102/selene-rover-repair-'+$stamp)))
foreach($target in @($backup,$output)){if(-not $target.StartsWith($root+'\',[StringComparison]::OrdinalIgnoreCase) -or (Test-Path -LiteralPath $target)){throw "Unsafe or existing output target: $target"}}
$before=(Get-FileHash -LiteralPath $map -Algorithm SHA256).Hash.ToLowerInvariant()
New-Item -ItemType Directory -Path $backup | Out-Null
New-Item -ItemType Directory -Path $output | Out-Null
$backupMap=Join-Path $backup 'MoonBase.umap'
[IO.File]::Copy($map,$backupMap,$false)
if((Get-FileHash -LiteralPath $backupMap -Algorithm SHA256).Hash.ToLowerInvariant() -ne $before){throw 'Map backup hash mismatch; editor not launched'}
$backupReceipt=[ordered]@{map='Content/Lunar/Maps/MoonBase.umap';before_sha256=$before;backup=$backupMap.Substring($root.Length+1);created_utc=[DateTime]::UtcNow.ToString('o');scope='Only existing MoonBase Lunokhod actor rotation and worldZ; no unrelated actors/maps/assets'}
$backupReceipt | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $output 'backup-receipt.json') -Encoding utf8
$receipt=Join-Path $output 'repair-receipt.json';$log=Join-Path $output 'editor-repair.log'
$configs=@('WindowsEditor','Windows') | ForEach-Object {$path=Join-Path $root ('Saved/Config/'+$_+'/GameUserSettings.ini');$exists=Test-Path -LiteralPath $path;[pscustomobject]@{Path=$path;Exists=$exists;Bytes=$(if($exists){[IO.File]::ReadAllBytes($path)}else{$null})}}
$argsList=@(('"'+(Join-Path $root 'LunarRescue.uproject')+'"'),'-run=pythonscript',('-script="'+$python+'"'),'-unattended','-nop4','-nosound','-nullrhi','-RepairSeleneRover',('-RepairReceipt="'+$receipt+'"'),('-RepairMapSHA='+$before),('-abslog="'+$log+'"'))
$started=[DateTime]::UtcNow
try{
 $process=Start-Process -FilePath $editor -ArgumentList $argsList -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(180000)){if(-not $process.HasExited){Stop-Process -Id $process.Id};$process.WaitForExit();throw "Repair timed out. Backup retained: $backupMap"}
 if($process.ExitCode -ne 0){throw "Editor repair failed exit$($process.ExitCode). Backup retained: $backupMap; log$log"}
}finally{
 foreach($config in $configs){if($config.Exists){[IO.File]::WriteAllBytes($config.Path,$config.Bytes)}elseif(Test-Path -LiteralPath $config.Path){Remove-Item -LiteralPath $config.Path}}
}
if(-not(Test-Path -LiteralPath $receipt) -or (Get-Item -LiteralPath $receipt).LastWriteTimeUtc -lt $started){throw "No fresh success receipt; inspect$log. Original backup retained at$backupMap"}
$result=Get-Content -LiteralPath $receipt -Raw | ConvertFrom-Json
$after=(Get-FileHash -LiteralPath $map -Algorithm SHA256).Hash.ToLowerInvariant()
if(-not $result.success -or $result.before_sha256 -ne $before -or $result.after_sha256 -ne $after -or -not $result.other_actor_snapshots_equal -or $result.clearance_min_cm -lt .99){throw 'Repair receipt failed semantic/hash checks; keep backup and inspect before recooking'}
if((Get-Content -LiteralPath $log -Raw) -notmatch 'SELENE_ROVER_REPAIR_COMPLETE'){throw 'No Editor repair completion marker'}
Write-Output "SELENE rover repair complete: $receipt"
Write-Output 'Recook/stage MoonBase, rerun affected rescue/interaction/movement checks and capture a new rover view before delivery. This helper does not launch a game, recook or claim runtime/render acceptance.'
