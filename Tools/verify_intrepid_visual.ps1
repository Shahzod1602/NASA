<##
Checks the retained Intrepid collision/resource invariants, or captures eight
fixed-camera Low/Medium PNGs. A fresh label is required for each invocation.
NullRHI checks provide no graphical performance or human playtest evidence.
-Cabin selects separate cabin evidence; -CabinBaseline disables only the cabin
finish. -Review -CabinReview captures two close views at Low/Medium (4 PNGs).
Examples:
  ./Tools/verify_intrepid_visual.ps1 -Packaged -Label intrepid-test-after
  ./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Baseline -Label intrepid-before-720
  ./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Label intrepid-after-1080 -Width 1920 -Height 1080
##>
param(
 [switch]$Packaged,[switch]$Review,[switch]$Rendered,[switch]$Baseline,
 [switch]$Cabin,[switch]$CabinBaseline,[switch]$CabinReview,
 [Alias('Run','IntrepidRun')][ValidatePattern('^[A-Za-z0-9_-]{1,64}$')][string]$Label='',
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720,
 [string]$EngineRoot='D:/UnrealEngine/UE_5.8',[string]$Python='python',
 [ValidateRange(30,900)][int]$TimeoutSeconds=300
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'IntrepidValidationCommon.ps1')
Invoke-IntrepidRun -Kind $(if($Review){'review'}else{'test'}) -Packaged ([bool]$Packaged) -Baseline ([bool]$Baseline) -Rendered ([bool]$Rendered) -Cabin ([bool]$Cabin) -CabinBaseline ([bool]$CabinBaseline) -CabinReview ([bool]$CabinReview) -Label $Label -Width $Width -Height $Height -EngineRoot $EngineRoot -Python $Python -TimeoutSeconds $TimeoutSeconds
