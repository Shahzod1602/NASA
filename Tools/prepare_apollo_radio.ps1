$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$out=Join-Path $root 'SourceArt\Apollo12\Radio'
New-Item -ItemType Directory -Force -Path $out | Out-Null
Add-Type -AssemblyName System.Speech
$speaker=New-Object System.Speech.Synthesis.SpeechSynthesizer
$speaker.SelectVoice('Microsoft Zira Desktop');$speaker.Rate=-1
$keys=@('StartMessage','CheckoutDone','InstrumentsDone','SamplesDone','SurveyorReached','PartsDone','MissionDone')
$strings=Get-Content (Join-Path $root 'Source\LunarRescue\LunarApolloStrings.inl') -Encoding UTF8
try {
 foreach($key in $keys){
  $line=$strings | Where-Object {$_ -match ('^APOLLO_TEXT\("'+$key+'", ')}
  $encoded=($line -replace '^APOLLO_TEXT\("[^"]+", ','') -replace '\)$',''
  $text=$encoded | ConvertFrom-Json
  $speaker.SetOutputToWaveFile((Join-Path $out ($key+'.wav')));$speaker.Speak($text);$speaker.SetOutputToNull()
 }
} finally {$speaker.Dispose()}
'Original synthesized English radio narration. Microsoft Zira local speech synthesis. Authored game dialogue, not NASA recordings or astronaut voices. Subtitles match the source String Table. No historical audio is claimed.' | Set-Content (Join-Path $out 'CREDITS.txt') -Encoding UTF8
