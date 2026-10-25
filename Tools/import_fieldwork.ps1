param([switch]$SkipStatic,[string]$EngineRoot='D:\UnrealEngine\UE_5.8')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
# Release the complete editor heap between static, skin and LOD imports.
$steps=@('import_suit.py','import_suit_rig.py','import_fieldwork.py')
if($SkipStatic){$steps=@('import_suit_rig.py','import_fieldwork.py')}
foreach($step in $steps){
 $log=Join-Path $root "Reports\fieldwork-$step.log"
 $arguments=@("`"$root\LunarRescue.uproject`"",'-run=pythonscript',"-script=`"$root\Tools\$step`"",'-unattended','-nop4','-nosound','-nullrhi',"-abslog=`"$log`"")
 if($step -eq 'import_fieldwork.py'){$arguments+='-realismfinishonly'}
 $process=Start-Process -FilePath "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(-not $process.WaitForExit(900000)){Stop-Process -Id $process.Id;throw "Import timeout: $step"}
 $marker=switch($step){'import_suit.py' {'SUIT_ASSETS_READY'} 'import_suit_rig.py' {'RIG_READY'} default {'APOLLO_FIELDWORK_ASSETS_READY'}}
 if($process.ExitCode -ne 0 -or (Get-Content -LiteralPath $log -Raw) -notmatch $marker){throw "Import failed: $step; see $log"}
 Write-Output "Fieldwork import complete: $step"
}
