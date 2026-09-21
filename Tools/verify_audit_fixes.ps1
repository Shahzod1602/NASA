param(
    [string]$EngineRoot = 'D:\UnrealEngine\UE_5.8',
    [switch]$Graphics
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $editor)) { throw "Engine not found: $editor" }
$tests = @('LunarTest','LunarMovementTest','LunarLedgeTest','LunarWalkthroughTest')
foreach ($test in $tests) {
    $logPath = Join-Path $projectRoot "Reports\fixed-$test.log"
    $arguments = @("`"$projectRoot\LunarRescue.uproject`"", '/Game/Lunar/Maps/MoonBase',
        '-game', '-unattended', "-$test", "-abslog=`"$logPath`"")
    if ($Graphics) {
        $arguments += @('-dx11','-RenderOffscreen','-ResX=1280','-ResY=720',
            '-ExecCmds="r.Streaming.PoolSize 384,r.ShadowQuality 2,t.MaxFPS 60"')
    } else { $arguments += @('-nullrhi','-nosound') }
    $process = Start-Process -FilePath $editor -ArgumentList $arguments -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(420000)) {
        Stop-Process -Id $process.Id
        throw "$test timed out; see $logPath"
    }
    if ($process.ExitCode -ne 0) { throw "$test failed with exit $($process.ExitCode); see $logPath" }
    Write-Output "$test PASS"
}
