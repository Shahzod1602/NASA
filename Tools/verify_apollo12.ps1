param([string]$EngineRoot='D:\UnrealEngine\UE_5.8',[switch]$Packaged,[switch]$Graphics,[switch]$Walkthrough,[switch]$HistoricalRoute,[switch]$Suit,[switch]$Checkpoints,[switch]$Polish,[switch]$Review,[switch]$Regression,[switch]$LegacyLanguage,[ValidateSet('en')][string]$Language='en',[int]$Width=1280,[int]$Height=720)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$env:TEMP=Join-Path $root 'Intermediate\ApolloTemp';$env:TMP=$env:TEMP
$exe=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Binaries\Win64\LunarRescue.exe'}else{Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'}
$cases=,@('/Game/Lunar/Maps/Apollo12','ApolloTest','checks')
if($Regression){$cases+=,@('/Game/Lunar/Maps/MoonBase','LunarTest','legacy-moon');$cases+=,@('/Game/Mars/Maps/MarsBase','LunarTest','mars');$cases+=,@('/Game/Rescue/Maps/MissionSelect','LunarMenuTest','menu')}
if($Walkthrough){$cases+=,@('/Game/Lunar/Maps/Apollo12','ApolloWalkTest','walk')}
if($HistoricalRoute){$cases+=,@('/Game/Lunar/Maps/Apollo12','ApolloWalkTest','historical-walk')}
if($Suit){$cases+=,@('/Game/Lunar/Maps/Apollo12','SuitTest','suit')}
if($Polish){$cases+=,@('/Game/Lunar/Maps/Apollo12','ApolloPolishTest','polish')}
if($Checkpoints){$cases+=,@('/Game/Lunar/Maps/Apollo12','ApolloSaveTest','save');$cases+=,@('/Game/Lunar/Maps/Apollo12','ApolloSaveWriteTest','save-write');$cases+=,@('/Game/Rescue/Maps/MissionSelect','ApolloSaveReadTest','save-read')}
if($Review){$cases=,@('/Game/Lunar/Maps/Apollo12','ApolloReview','visuals');$Graphics=$true}
foreach($case in $cases){
 $prefix=if($Packaged){'package'}else{'editor'}
 $logPath=Join-Path $root "Reports\$prefix-apollo12-$($case[2]).log"
 $resultName=switch($case[1]){
  'ApolloTest' {'apollo12-checks.txt'}
  'ApolloWalkTest' {'apollo12-walkthrough.txt'}
  'SuitTest' {'suit-checks.txt'}
  'ApolloPolishTest' {'polish-checks.txt'}
  'ApolloSaveTest' {'checkpoint-checks.txt'}
  'ApolloSaveWriteTest' {'checkpoint-write-checks.txt'}
  'ApolloSaveReadTest' {'checkpoint-read-checks.txt'}
  'LunarMenuTest' {'menu-travel-checks.txt'}
  'LunarTest' {if($case[0] -like '*Mars*'){'mars-gameplay-checks.txt'}else{'gameplay-checks.txt'}}
 }
 $resultRoot=if($Packaged){Join-Path $root 'Dist\Apollo12\Windows\LunarRescue\Reports'}else{Join-Path $root 'Reports'}
 $reviewNames=@('briefing','intrepid','alsep','sample','surveyor','map','sources','result','approach','hardware','return','history','evidence','tutorial','work-prompt','work-progress','continue','new-confirm','reload','retry-checkpoint','recovered','tool-level','tool-antenna','tool-core','tool-camera','settings','album','text150')
 if($case[1] -eq 'ApolloReview'){
  foreach($name in $reviewNames){$imagePath=Join-Path $resultRoot "apollo12-$name.png";if(Test-Path -LiteralPath $imagePath){Remove-Item -LiteralPath $imagePath}}
 }
 $resultFile=if($resultName){Join-Path $resultRoot $resultName}
 if($resultFile -and (Test-Path -LiteralPath $resultFile)){Remove-Item -LiteralPath $resultFile}
 $arguments=@($case[0],'-game','-unattended',"-$($case[1])","-abslog=`"$logPath`"")
 # An obsolete launcher argument must not restore the former Uzbek UI.
 if($LegacyLanguage){$arguments+='-ApolloLanguage=uz'}
 if($case[2] -eq 'historical-walk'){$arguments+='-ApolloHistoricalRoute'}
 if(-not $Packaged){$arguments=@("`"$root\LunarRescue.uproject`"")+$arguments}
 if($Graphics){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+=@('-nullrhi','-nosound')}
 # Walkthrough uses fixed 1/60-second simulation steps; uncapped rendering accelerates the test.
 # Resource time and character movement remain normal game values, not teleports or cheats.
 if($case[1] -eq 'ApolloWalkTest'){$arguments+=@('-benchmark','-fps=60','-ExecCmds="t.MaxFPS 0"')}
 $p=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
 if(-not $p.WaitForExit(1800000)){Stop-Process -Id $p.Id;throw "$($case[2]) timed out; see $logPath"}
 Write-Output "$prefix Apollo12 $($case[2]): exit $($p.ExitCode) (PID $($p.Id))"
 if($p.ExitCode -ne 0){throw "$($case[2]) failed; see $logPath"}
 # Unreal's graceful shutdown may return zero even when a check requests failure.
 # Read the fresh semantic result instead of accepting process exit alone.
 if($resultFile){
  if(-not(Test-Path -LiteralPath $resultFile)){throw "$($case[2]) produced no result: $resultFile"}
  $resultText=Get-Content -LiteralPath $resultFile -Raw
  if($resultText -notmatch '(?m)^RESULT: 0 failures\r?$' -or $resultText -match '(?m)^FAIL '){throw "$($case[2]) reported a failed check; see $resultFile"}
  Copy-Item -LiteralPath $resultFile -Destination (Join-Path $root "Reports\$prefix-apollo12-$($case[2])-result.txt")
 }
 if($case[1] -eq 'ApolloReview'){
  foreach($name in $reviewNames){
   $imagePath=Join-Path $resultRoot "apollo12-$name.png"
   if(-not(Test-Path -LiteralPath $imagePath)){throw "Missing fresh review screenshot: $imagePath"}
   $png=[System.IO.File]::ReadAllBytes($imagePath)
   if($png.Length -lt 24 -or [System.Text.Encoding]::ASCII.GetString($png,1,3) -ne 'PNG'){throw "Invalid screenshot: $imagePath"}
   $actualWidth=[System.Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($png,16))
   $actualHeight=[System.Net.IPAddress]::NetworkToHostOrder([BitConverter]::ToInt32($png,20))
   if($actualWidth -ne $Width -or $actualHeight -ne $Height){throw "Screenshot is $($actualWidth)x$actualHeight instead of $($Width)x$Height : $imagePath"}
  }
  Write-Output "Validated $($reviewNames.Count) fresh screenshots at $($Width)x$Height ($Language)"
 }
}
