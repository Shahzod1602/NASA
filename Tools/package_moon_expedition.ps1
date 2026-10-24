param([string]$EngineRoot='D:\UnrealEngine\UE_5.8')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force -Path "$root\Intermediate\MissionTemp" | Out-Null
$env:TEMP="$root\Intermediate\MissionTemp";$env:TMP=$env:TEMP
& "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=$root\LunarRescue.uproject" -noP4 -platform=Win64 -clientconfig=Development -build -cook -map=/Game/Rescue/Maps/MissionSelect+/Game/Lunar/Maps/MoonBase+/Game/Mars/Maps/MarsBase -stage -pak -archive "-archivedirectory=$root\Dist\MoonExpedition" -unattended -utf8output '-UbtArgs=-MaxParallelActions=1 -NoUBA'
if($LASTEXITCODE -ne 0){throw "Expedition package failed: $LASTEXITCODE"}
$sourceArt=Join-Path $root 'SourceArt'
$creditsRoot=Join-Path $root 'Dist\MoonExpedition\Windows\Credits'
Get-ChildItem -LiteralPath $sourceArt -Recurse -File | Where-Object { $_.Name -like '*CREDITS*' -or $_.Name -like '*LICENSE*' } | ForEach-Object {
 $destination=Join-Path $creditsRoot $_.FullName.Substring($sourceArt.Length).TrimStart('\')
 New-Item -ItemType Directory -Force -Path (Split-Path $destination -Parent) | Out-Null
 Copy-Item -LiteralPath $_.FullName -Destination $destination
}
Write-Output "Expedition package: $root\Dist\MoonExpedition\Windows\LunarRescue.exe"
