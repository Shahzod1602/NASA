param([string]$EngineRoot = 'D:\UnrealEngine\UE_5.8')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$automation = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path -LiteralPath $automation)) { throw "Engine not found: $automation" }
& $automation BuildCookRun "-project=$projectRoot\LunarRescue.uproject" -noP4 -platform=Win64 `
    -clientconfig=Development -build -cook -map=/Game/Lunar/Maps/MoonBase -stage -pak -archive `
    "-archivedirectory=$projectRoot\Dist" -unattended -utf8output '-UbtArgs=-MaxParallelActions=2'
if ($LASTEXITCODE -ne 0) { throw "Packaging failed: $LASTEXITCODE" }
$sourceArt = Join-Path $projectRoot 'SourceArt'
$creditsRoot = Join-Path $projectRoot 'Dist\Windows\Credits'
Get-ChildItem -LiteralPath $sourceArt -Recurse -File |
    Where-Object { $_.Name -like '*CREDITS*' -or $_.Name -like '*LICENSE*' } |
    ForEach-Object {
        $relative = $_.FullName.Substring($sourceArt.Length).TrimStart('\')
        $destination = Join-Path $creditsRoot $relative
        New-Item -ItemType Directory -Force -Path (Split-Path $destination -Parent) | Out-Null
        Copy-Item -LiteralPath $_.FullName -Destination $destination
    }
