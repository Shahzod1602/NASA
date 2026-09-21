# Synthesizes original ARES dialogue locally; then applies the radio filter once.
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force (Join-Path $root 'SourceArt\Mars') | Out-Null
Add-Type -AssemblyName System.Speech
$voice=New-Object System.Speech.Synthesis.SpeechSynthesizer
try {
 $voice.SelectVoice('Microsoft Zira Desktop')
 $lines=@{
  StoryIntro='ARES is silent. A dust storm damaged the station power and relay.'
  StoryStart='Navigation. Recover the spare power cell beside the survey rover.'
  Module='Module secured. Install it in the external power port.'
  StoryPower='Power restored. Recover the subsurface ice survey from the science terminal.'
  StoryData='Data recovered. Align the orbital antenna before returning to the lander.'
  StoryTruth='Crew log. We sheltered in the canyon. The ice samples are safe. Relay needed for pickup.'
  StoryWin='Signal restored. We have your signal. Bring yourself home.'
  StoryWarning='Caution. One minute of oxygen. Return to the lander.'
 }
 foreach($key in $lines.Keys){$voice.SetOutputToWaveFile((Join-Path $root "SourceArt\Mars\Radio_$key.wav"));$voice.Speak($lines[$key])}
} finally {$voice.Dispose()}
python (Join-Path $PSScriptRoot 'filter_mars_audio.py')
if($LASTEXITCODE -ne 0){throw 'Radio filtering failed'}
