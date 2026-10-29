param([switch]$Packaged,[string]$Label='current',[ValidateRange(-1,2)][int]$Quality=-1)
$ErrorActionPreference='Stop'
if($Label -notmatch '^[A-Za-z0-9_-]+$'){throw 'Invalid review label'}
$root=Split-Path $PSScriptRoot -Parent
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
$folder=Join-Path $root "Reports\Realism\$Label"
New-Item -ItemType Directory -Force -Path $folder | Out-Null
$reportRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Reports'}else{Join-Path $root 'Reports'}
$exe=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Binaries\Win64\LunarRescue.exe'}else{'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$names=@('hands','body','third-person','front','work','walk','stride','hop')
foreach($name in $names){$path=Join-Path $reportRoot "suit-$name.png";if(Test-Path -LiteralPath $path){Copy-Item -LiteralPath $path -Destination (Join-Path $folder "previous-$name.png");Remove-Item -LiteralPath $path}}
$argsList=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-SuitReview','-dx11','-RenderOffscreen','-ForceRes','-ResX=1280','-ResY=720','-nosound',"-abslog=`"$folder\capture.log`"")
if(-not $Packaged){$argsList=@("`"$root\LunarRescue.uproject`"")+$argsList}
if($Quality -ge 0){$argsList+="-ApolloQuality=$Quality"}
$config=Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Saved\Config\Windows\GameUserSettings.ini'
if(-not $Packaged){$config=Join-Path $root 'Saved\Config\WindowsEditor\GameUserSettings.ini'}
$hadConfig=Test-Path -LiteralPath $config
$configBytes=if($hadConfig){[System.IO.File]::ReadAllBytes($config)}else{$null}
try {
$p=Start-Process -FilePath $exe -ArgumentList $argsList -WindowStyle Hidden -PassThru
if(-not $p.WaitForExit(120000)){Stop-Process -Id $p.Id;throw 'Suit beauty review timed out'}
} finally {
 if($hadConfig){[System.IO.File]::WriteAllBytes($config,$configBytes)}
 elseif(Test-Path -LiteralPath $config){Remove-Item -LiteralPath $config}
}
$logText=Get-Content -LiteralPath (Join-Path $folder 'capture.log') -Raw
if($logText -match 'Failed to compile Material|Fatal error:|Unhandled Exception:'){throw 'Render used a failed material or crashed; inspect capture.log'}
foreach($name in $names){
 $path=Join-Path $reportRoot "suit-$name.png"
 if(-not(Test-Path -LiteralPath $path)){throw "No fresh $name frame"}
 Copy-Item -LiteralPath $path -Destination (Join-Path $folder "$name.png")
}
Write-Output "Eight fresh real rendered frames saved in $folder"
