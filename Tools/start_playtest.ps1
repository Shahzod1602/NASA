param([Parameter(Mandatory=$true)][ValidatePattern('^P0[1-8]$')][string]$Participant,[Parameter(Mandatory=$true)][string]$Archive)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$zip=(Resolve-Path -LiteralPath $Archive).Path
$destination=Join-Path $root "Dist\Playtests\$Participant"
if(Test-Path -LiteralPath $destination){throw 'This participant already has a test installation. Use it to resume; do not overwrite the existing evidence.'}
New-Item -ItemType Directory -Path $destination | Out-Null
Expand-Archive -LiteralPath $zip -DestinationPath $destination
$exes=@(Get-ChildItem -LiteralPath $destination -Filter LunarRescue.exe -File -Recurse | Where-Object {$_.FullName -notlike '*\Binaries\*'})
if($exes.Count -ne 1){throw 'Expected exactly one game bootstrap in the release archive'}
Write-Output "Anonymous participant $Participant; isolated save and photos in $destination"
Start-Process -FilePath $exes[0].FullName -WorkingDirectory $exes[0].DirectoryName
