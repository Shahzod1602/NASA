param([string]$EngineRoot='D:\UnrealEngine\UE_5.8',[switch]$Graphics,[switch]$Walkthrough,[switch]$Packaged)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$exe=if($Packaged){Join-Path $root 'Dist\Windows\LunarRescue\Binaries\Win64\LunarRescue.exe'}else{Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$cases=@(
 @('/Game/Rescue/Maps/MissionSelect','LunarMenuTest','menu'),
 @('/Game/Lunar/Maps/MoonBase','LunarTest','moon-gameplay'),
 @('/Game/Mars/Maps/MarsBase','LunarTest','mars-gameplay'),
 @('/Game/Mars/Maps/MarsBase','LunarMovementTest','mars-movement'),
 @('/Game/Mars/Maps/MarsBase','LunarLedgeTest','mars-ledge'))
if($Walkthrough){$cases+=,@('/Game/Lunar/Maps/MoonBase','LunarWalkthroughTest','moon-walk');$cases+=,@('/Game/Mars/Maps/MarsBase','LunarWalkthroughTest','mars-walk')}
foreach($case in $cases){
 $prefix=if($Packaged){'package'}else{'editor'}
 $args=@($case[0],'-game','-unattended',"-$($case[1])","-abslog=$root\Reports\$prefix-$($case[2]).log")
 if(-not $Packaged){$args=@("`"$root\LunarRescue.uproject`"")+$args}
 if($Graphics){$args+=@('-dx11','-RenderOffscreen','-ResX=1280','-ResY=720')}else{$args+=@('-nullrhi','-nosound')}
 $p=Start-Process -FilePath $exe -ArgumentList $args -WindowStyle Hidden -PassThru
 if(-not $p.WaitForExit(600000)){Stop-Process -Id $p.Id;throw "$($case[2]) timed out"}
 Write-Output "$($case[2]): exit $($p.ExitCode)"
 if($p.ExitCode -ne 0){throw "$($case[2]) failed"}
}
exit 0
