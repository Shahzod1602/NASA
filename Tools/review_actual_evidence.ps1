$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$game=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue'
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
$checkpoint=Join-Path $game 'Saved\AutomatedTests\ApolloProfile\Apollo12.sav'
if(-not(Test-Path -LiteralPath $checkpoint)){throw 'Complete the full rendered profile before reviewing its actual evidence'}
$before=(Get-FileHash -LiteralPath $checkpoint -Algorithm SHA256).Hash
$reviewDirectory=Join-Path $game 'Saved\AutomatedTests\ApolloCrossProcess'
New-Item -ItemType Directory -Force -Path $reviewDirectory | Out-Null
$reviewSnapshot=Join-Path $reviewDirectory 'Apollo12.sav'
$reviewExisted=Test-Path -LiteralPath $reviewSnapshot
$reviewBytes=if($reviewExisted){[System.IO.File]::ReadAllBytes($reviewSnapshot)}else{$null}
# Performance runs deliberately skip checkpoint reads. Review a byte-identical
# copy through the existing read-only automation slot, never the player's slot.
Copy-Item -LiteralPath $checkpoint -Destination $reviewSnapshot -Force
try {
$result=Join-Path $game 'Reports\actual-evidence-checks.txt'
if(Test-Path -LiteralPath $result){Remove-Item -LiteralPath $result}
$log=Join-Path $root 'Reports\v050-actual-evidence.log'
$arguments=@('/Game/Lunar/Maps/Apollo12?ResumeApollo=1','-game','-unattended','-ApolloSaveMenuReview','-ApolloEvidenceReview','-dx11','-RenderOffscreen','-ForceRes','-ResX=1280','-ResY=720','-nosound',"-abslog=`"$log`"")
$p=Start-Process -FilePath (Join-Path $game 'Binaries\Win64\LunarRescue.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru
if(-not $p.WaitForExit(60000)){Stop-Process -Id $p.Id;throw 'Actual evidence review timed out'}
if(-not(Test-Path -LiteralPath $result)){throw 'No fresh historical-resume result'}
$text=Get-Content -LiteralPath $result -Raw
if($text -match '(?m)^FAIL ' -or $text -notmatch '(?m)^RESULT: 0 failures\r?$'){throw 'Historical resume or actual photograph decoding failed'}
if((Get-FileHash -LiteralPath $checkpoint -Algorithm SHA256).Hash -ne $before){throw 'Read-only evidence review modified the checkpoint'}
if((Get-FileHash -LiteralPath $reviewSnapshot -Algorithm SHA256).Hash -ne $before){throw 'Read-only evidence review modified its copied snapshot'}
Copy-Item -LiteralPath $result -Destination (Join-Path $root 'Reports\v050-actual-evidence-result.txt')
Write-Output $text
Write-Output 'Actual historical checkpoint remained byte-for-byte unchanged.'
} finally {
 if($reviewExisted){[System.IO.File]::WriteAllBytes($reviewSnapshot,$reviewBytes)}
 elseif(Test-Path -LiteralPath $reviewSnapshot){Remove-Item -LiteralPath $reviewSnapshot}
}
