param([string]$EngineRoot='D:\UnrealEngine\UE_5.8',[switch]$Walkthrough,[switch]$Graphics,[switch]$Packaged,[switch]$Review)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$exe=if($Packaged){Join-Path $root 'Dist\MoonExpedition\Windows\LunarRescue\Binaries\Win64\LunarRescue.exe'}else{Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$cases=@(@('/Game/Lunar/Maps/MoonBase','LunarTest','expedition-gameplay'),@('/Game/Mars/Maps/MarsBase','LunarTest','expedition-mars-regression'),@('/Game/Rescue/Maps/MissionSelect','LunarMenuTest','expedition-menu'))
if($Walkthrough){$cases+=,@('/Game/Lunar/Maps/MoonBase','LunarWalkthroughTest','expedition-walk')}
if($Review){$cases=,@('/Game/Lunar/Maps/MoonBase','LunarExpeditionReview','expedition-visuals');$Graphics=$true}
foreach($case in $cases){
 if($case -is [string] -or $case.Count -ne 3 -or -not $case[0].StartsWith('/Game/')){throw 'Invalid test case: expected map, flag and report name'}
 $prefix=if($Packaged){'package'}else{'editor'}
 $logPath="$root\Reports\$prefix-$($case[2]).log"
 $arguments=@($case[0],'-game','-unattended',"-$($case[1])","-abslog=`"$logPath`"")
 if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
 if($Graphics){$arguments+=@('-dx11','-RenderOffscreen','-ResX=1280','-ResY=720')}else{$arguments+=@('-nullrhi','-nosound')}
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(600000)){Stop-Process -Id $process.Id;throw "$($case[2]) timed out; see $logPath"}
 Write-Output "$($case[2]): exit $($process.ExitCode)"
 if($process.ExitCode -ne 0){throw "$($case[2]) failed; see $logPath"}
}
