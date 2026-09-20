Add-Type -AssemblyName System.Speech
$speaker = New-Object System.Speech.Synthesis.SpeechSynthesizer
$speaker.SelectVoice('Microsoft Zira Desktop')
$speaker.Rate = 0
$lines = @{
    StoryIntro = 'Signal lost. Selene, do you copy? There should be someone here.'
    StoryStart = 'Find the rover power cell. Selene has stopped responding.'
    StoryPower = 'Power restored. Crew log: The rover did not fail. We shut it down.'
    StoryData = 'Data recovered. A crew recorder is on the survey ledge. Q selects the optional route.'
    StoryTruth = 'Crew log: Cooling failed. We cut rover power to save the samples. Crew safe at the relay.'
    StoryWin = 'We have your signal. Bring yourself home.'
    StoryWarning = 'Caution. One minute of oxygen. Return to the lander.'
}
foreach ($item in $lines.GetEnumerator()) {
    $speaker.SetOutputToWaveFile("D:\LunarRescue\SourceArt\Immersion\Radio_$($item.Key).wav")
    $speaker.Speak($item.Value)
    $speaker.SetOutputToNull()
}
$speaker.Dispose()
Write-Output 'Seven English story radio clips generated.'
