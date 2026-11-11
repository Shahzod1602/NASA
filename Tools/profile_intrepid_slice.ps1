<##
Captures the Intrepid driver's four fixed cameras, with the new art enabled or
opted out in the same binary. Medium quality and production parked props are
explicit. The driver settles 300 frames before CSV and owns capture/exit.
Do not compare this directly with profile_apollo12's moving opening workload.
-Cabin keeps these same four cameras while using separate cabin reports;
-CabinBaseline opts out only the new cabin, independently of -Baseline.
Examples (run sequentially; no builds or other Unreal instances):
  ./Tools/profile_intrepid_slice.ps1 -Packaged -Baseline -Label intrepid-control-q1 -GpuTelemetry
  ./Tools/profile_intrepid_slice.ps1 -Packaged -Label intrepid-art-q1 -GpuTelemetry
##>
param(
 [switch]$Packaged,[switch]$Baseline,[switch]$Cabin,[switch]$CabinBaseline,
 [Alias('Run','IntrepidRun')][ValidatePattern('^[A-Za-z0-9_-]{1,64}$')][string]$Label='',
 [ValidateRange(1200,12000)][int]$Frames=6000,
 [ValidateRange(640,3840)][int]$Width=1280,
 [ValidateRange(480,2160)][int]$Height=720,
 [ValidateRange(0,240)][int]$FpsCap=60,[switch]$GpuTelemetry,
 [string]$EngineRoot='D:/UnrealEngine/UE_5.8',[string]$Python='python',
 [ValidateRange(30,900)][int]$TimeoutSeconds=720
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'IntrepidValidationCommon.ps1')
Invoke-IntrepidRun -Kind profile -Packaged ([bool]$Packaged) -Baseline ([bool]$Baseline) -Rendered $true -Cabin ([bool]$Cabin) -CabinBaseline ([bool]$CabinBaseline) -Label $Label -Width $Width -Height $Height -Frames $Frames -FpsCap $FpsCap -GpuTelemetry ([bool]$GpuTelemetry) -EngineRoot $EngineRoot -Python $Python -TimeoutSeconds $TimeoutSeconds
