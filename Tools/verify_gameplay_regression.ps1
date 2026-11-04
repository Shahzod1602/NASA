$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$folder=Join-Path $root ('Reports/Gameplay20261103/final-queue-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $folder|Out-Null
$cases=@(
 @{Name='gameplay-functional';Script='verify_gameplay_tasks.ps1';Parameters=@{Packaged=$true}},
 @{Name='gameplay-rendered';Script='verify_gameplay_tasks.ps1';Parameters=@{Packaged=$true;Rendered=$true}},
 @{Name='gameplay-review720';Script='verify_gameplay_tasks.ps1';Parameters=@{Packaged=$true;Review=$true;Width=1280;Height=720}},
 @{Name='gameplay-review1080';Script='verify_gameplay_tasks.ps1';Parameters=@{Packaged=$true;Review=$true;Width=1920;Height=1080}},
 @{Name='apollo-regressions';Script='verify_apollo12.ps1';Parameters=@{Packaged=$true;Regression=$true;Checkpoints=$true;Polish=$true;Suit=$true;Guidance=$true;Fieldwork=$true;Walkthrough=$true}},
 @{Name='nasa-regressions';Script='verify_nasa_tasks.ps1';Parameters=@{Packaged=$true}},
 @{Name='selection';Script='verify_mission_selection.ps1';Parameters=@{Packaged=$true}},
 @{Name='frontend';Script='verify_frontend.ps1';Parameters=@{Packaged=$true}},
 @{Name='experience';Script='verify_experience.ps1';Parameters=@{Packaged=$true}},
 @{Name='presentation';Script='verify_presentation.ps1';Parameters=@{Packaged=$true;Graphics=$true;Audio=$true}},
 @{Name='movement';Script='verify_planet_movement.ps1';Parameters=@{Packaged=$true}},
 @{Name='equipment';Script='verify_feedback.ps1';Parameters=@{Packaged=$true}},
 @{Name='loading-font';Script='verify_loading_typography.ps1';Parameters=@{Packaged=$true}},
 @{Name='loading-travel';Script='verify_loading.ps1';Parameters=@{Packaged=$true;Graphics=$true}},
 @{Name='hud-review720';Script='verify_feedback.ps1';Parameters=@{Packaged=$true;Review=$true;Width=1280;Height=720}},
 @{Name='hud-review1080';Script='verify_feedback.ps1';Parameters=@{Packaged=$true;Review=$true;Width=1920;Height=1080}}
)
$results=[Collections.Generic.List[object]]::new()
$exe=Join-Path $root 'Dist/Apollo12/Windows/LunarRescue/Binaries/Win64/LunarRescue.exe'
$sha=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
foreach($case in $cases){
 $start=[DateTime]::UtcNow;$good=$false;$detail=$null;$log=Join-Path $folder ($case.Name+'.log')
 try{$params=$case.Parameters;& (Join-Path $root ('Tools/'+$case.Script)) @params *> $log;$good=$true}catch{$detail=$_.Exception.Message;$detail|Add-Content -LiteralPath $log}
 $results.Add([pscustomobject]@{name=$case.Name;script=$case.Script;parameters=$case.Parameters;started_utc=$start.ToString('o');finished_utc=[DateTime]::UtcNow.ToString('o');passed=$good;error=$detail;log=$log})
 [pscustomobject]@{executable_sha256=$sha;cases=$results;completed=$false}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $folder 'queue-summary.json') -Encoding utf8
 Write-Output "$($case.Name): $good"
}
$bad=@($results|Where-Object{-not $_.passed}).Count
[pscustomobject]@{executable_sha256=$sha;cases=$results;completed=$true;failures=$bad}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $folder 'queue-summary.json') -Encoding utf8
Write-Output "Validation queue: $folder"
if($bad){throw "$bad validation cases failed; inspect queue-summary.json"}
