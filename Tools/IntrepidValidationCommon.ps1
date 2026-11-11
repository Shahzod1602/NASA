# Shared runner; dot-source from the two public Intrepid wrappers.
function Get-IntrepidBusyProcesses([int]$IgnoreId=0){
 $native=@(Get-Process -Name LunarRescue,UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker,cl,link -ErrorAction SilentlyContinue | Where-Object {$_.Id -ne $IgnoreId})
 $managed=@(Get-CimInstance Win32_Process -Filter "Name='dotnet.exe'" | Where-Object {$_.ProcessId -ne $IgnoreId -and $_.CommandLine -match 'UnrealBuildTool|AutomationTool|BuildCookRun'})
 return @($native)+@($managed)
}
function Invoke-IntrepidRun {
 param(
  [ValidateSet('test','review','profile')][string]$Kind,
  [bool]$Packaged,[bool]$Baseline,[bool]$Rendered,
  [bool]$Cabin=$false,[bool]$CabinBaseline=$false,[bool]$CabinReview=$false,
  [string]$Label,[int]$Width,[int]$Height,[int]$Frames=6000,[int]$FpsCap=60,
  [bool]$GpuTelemetry=$false,
  [string]$EngineRoot='D:/UnrealEngine/UE_5.8',
  [string]$Python='python',
  [int]$TimeoutSeconds=720
 )
 $ErrorActionPreference='Stop'
 if($CabinReview -and $Kind -ne 'review'){throw 'CabinReview requires the Review mode; it is not a profile or functional test.'}
 if($CabinBaseline -or $CabinReview){$Cabin=$true}
 if(-not $Label){$Label=$Kind+'-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff')+'-'+$PID}
 if($Label -notmatch '^[A-Za-z0-9_-]{1,64}$'){throw 'Label must be 1-64 ASCII letters, digits, underscores or hyphens.'}
 if($Kind -eq 'profile' -and ($Frames%4 -ne 0)){throw 'Profile frames must be divisible by four for equal fixed-camera phases.'}
 if($Kind -in @('review','profile')){$Rendered=$true}
 $root=[IO.Path]::GetFullPath((Split-Path $PSScriptRoot -Parent))
 $projectRoot=if($Packaged){Join-Path $root 'Dist/Apollo12/Windows/LunarRescue'}else{$root}
 $editor=if($Rendered){'UnrealEditor.exe'}else{'UnrealEditor-Cmd.exe'}
 $exe=if($Packaged){Join-Path $projectRoot 'Binaries/Win64/LunarRescue.exe'}else{Join-Path $EngineRoot ('Engine/Binaries/Win64/'+$editor)}
 $module=if($Packaged){$null}else{Join-Path $root 'Binaries/Win64/UnrealEditor-LunarRescue.dll'}
 $helper=Join-Path $PSScriptRoot 'intrepid_validation.py'
 $family=if($Cabin){'Reports/IntrepidCabin20261111'}else{'Reports/IntrepidVisual20261111'}
 $controller=Join-Path $root ($family+'/runs/'+$Label)
 $driver=Join-Path $projectRoot ($family+'/'+$Label)
 foreach($path in @($exe,$helper)){if(-not(Test-Path -LiteralPath $path -PathType Leaf)){throw "Missing run input: $path"}}
 if($module -and -not(Test-Path -LiteralPath $module -PathType Leaf)){throw "Missing editor game module: $module"}
 if((Test-Path -LiteralPath $controller) -or (Test-Path -LiteralPath $driver)){throw 'Run label already exists; choose a fresh unique label.'}
 if(@(Get-IntrepidBusyProcesses).Count){throw 'Game/editor/shader/compiler/build work is running; wait before an isolated Intrepid run.'}
 # Python verifies containment/reparse ancestors before the controller folder is made.
 $fingerprintBefore=Join-Path $controller 'fingerprint-before.json'
 $fingerprintArguments=@($helper,'fingerprint','--root',$root,'--exe',$exe,'--output',$fingerprintBefore)
 if($module){$fingerprintArguments+=@('--module',$module)}
 & $Python @fingerprintArguments | Out-Null
 if($LASTEXITCODE -ne 0){throw 'Unable to fingerprint executable/module/source/art inputs safely.'}
 $snapshot=Join-Path $controller 'preservation'
 & $Python $helper snapshot --root $root --output $snapshot | Out-Null
 if($LASTEXITCODE -ne 0){throw 'Protected files could not be backed up and checksummed; Unreal was not started.'}
 $snapshotManifest=Join-Path $snapshot 'manifest.json'
 $log=Join-Path $controller 'run.log'
 $arguments=@('/Game/Lunar/Maps/Apollo12','-game','-unattended','-nosound',('-IntrepidVisual'+$Kind.Substring(0,1).ToUpperInvariant()+$Kind.Substring(1)),"-IntrepidRun=$Label",'-FieldOperationsProfile',"-abslog=`"$log`"")
 if(-not $Packaged){$arguments=@("`"$root/LunarRescue.uproject`"")+$arguments+@('-DDC=InstalledNoZenLocalFallback')}
 if($Baseline){$arguments+='-IntrepidVisualBaseline'}
 if($Cabin){$arguments+='-IntrepidCabinScope'}
 if($CabinBaseline){$arguments+='-IntrepidCabinBaseline'}
 if($CabinReview){$arguments+='-IntrepidCabinReview'}
 if($Rendered){$arguments+=@('-dx11','-RenderOffscreen','-ForceRes',"-ResX=$Width","-ResY=$Height")}else{$arguments+='-nullrhi'}
 if($Kind -eq 'profile'){
  # The driver owns the finite capture and exit callback. Never add csvCaptureFrames/ExitAfterCsvProfiling.
  $arguments+=@("-IntrepidProfileFrames=$Frames",'-csvGpuStats','-ApolloQuality=1',"-ExecCmds=`"t.MaxFPS $FpsCap,r.VSync 0`"")
 }elseif($Kind -eq 'review'){
  # The driver explicitly applies Q0/Q1; a CLI ApolloQuality override would invalidate that comparison.
  $arguments+='-ExecCmds="t.MaxFPS 60,r.VSync 0"'
 }
 $before=Get-Content -LiteralPath $fingerprintBefore -Raw|ConvertFrom-Json
 $sourceCommit=((& git -C $root rev-parse HEAD) -join '').Trim()
 $dirty=@(& git -C $root status --porcelain)
 $environment=[ordered]@{
  schema_version=1;kind=$Kind;label=$Label;baseline=[bool]$Baseline;packaged=[bool]$Packaged
  cabin_scope=[bool]$Cabin;cabin_baseline=[bool]$CabinBaseline;cabin_review=[bool]$CabinReview
  human_playtest=$false;synthetic=$true;performance_measurement=($Kind -eq 'profile')
  renderer=$(if($Rendered){'DX11 / rendered offscreen'}else{'NullRHI / functional checks only'})
  width=$Width;height=$Height;quality=$(if($Kind -eq 'review'){'driver-applied Low and Medium'}elseif($Kind -eq 'profile'){1}else{'not a graphical measurement'})
  requested_profile_frames=$(if($Kind -eq 'profile'){$Frames}else{$null});settle_frames_before_csv=$(if($Kind -eq 'profile'){300}else{$null})
  fps_cap=$(if($Kind -eq 'profile'){$FpsCap}elseif($Kind -eq 'review'){60}else{$null});vsync=$false;audio=$false
  field_operations_requested=$true;gpu_csv_counters_requested=($Kind -eq 'profile');external_nvidia_telemetry_requested=[bool]$GpuTelemetry
  gpu_counter_note='Requested counters do not establish availability; inspect finite/positive counts in summary.json. Working set is not VRAM.'
  cpu=@((Get-CimInstance Win32_Processor).Name);adapters=@((Get-CimInstance Win32_VideoController).Name);ram_bytes=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
  source_commit=$sourceCommit;source_dirty=($dirty.Count -gt 0);git_status=$dirty
  executable_sha256=$before.executable.sha256;game_module_sha256=$(if($before.module){$before.module.sha256}else{$null});source_art_inputs_sha256=$before.input_sha256
  command_executable=$exe;command_arguments=$arguments;driver_output=$driver;controller_output=$controller
  memory_method='Maximum of one-second process working-set samples; not true peak, VRAM or dedicated GPU memory'
  comparison_scope=$(if($Cabin){'CabinBaseline toggles only the cabin derivative/shared white finish; the old VisualBaseline independently controls Nov11 surface/foil. Profile keeps the same four cameras including an outside-ROI control; close review uses two FOV60 cameras, not performance.'}else{'Baseline toggles the Nov11 surface/foil off in the same final binary. The four-camera workload includes one outside ROI control; it is not the generic opening route, occupied driving or whole-game performance.'})
 }
 $samples=[Collections.Generic.List[object]]::new();$gpuSamples=[Collections.Generic.List[string]]::new()
 $capture=$null;$completed=$false;$overlap=$false;$errorText=$null;$restoreError=$null;$maximumWorkingSet=0L;$tick=0
 $gpuCommand=if($GpuTelemetry){Get-Command nvidia-smi -ErrorAction SilentlyContinue}else{$null}
 $started=[DateTime]::UtcNow;$environment.started_utc=$started.ToString('o')
 try {
  if(@(Get-IntrepidBusyProcesses).Count){throw 'Competing work started during snapshot preparation; Unreal was not started.'}
  $capture=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
  $environment.process_id=$capture.Id
  while(-not $capture.WaitForExit(1000)){
   if([DateTime]::UtcNow -gt $started.AddSeconds($TimeoutSeconds)){throw "Intrepid $Kind exceeded $TimeoutSeconds seconds; only this wrapper's process will be stopped."}
   $capture.Refresh();$maximumWorkingSet=[Math]::Max($maximumWorkingSet,$capture.WorkingSet64)
   $samples.Add([ordered]@{elapsed_seconds=([DateTime]::UtcNow-$started).TotalSeconds;working_set_bytes=$capture.WorkingSet64})
   if(@(Get-IntrepidBusyProcesses -IgnoreId $capture.Id).Count){$overlap=$true}
   ++$tick
   if($gpuCommand -and $tick%5 -eq 0){
    $reading=& $gpuCommand.Source --query-gpu=timestamp,name,temperature.gpu,utilization.gpu,clocks.gr,clocks.mem,power.draw --format=csv,noheader,nounits 2>$null
    if($LASTEXITCODE -eq 0){foreach($line in $reading){if($line){$gpuSamples.Add([string]$line)}}}
   }
  }
  $capture.WaitForExit();$capture.Refresh()
  $environment.process_exit_code=$capture.ExitCode
  if($capture.ExitCode -ne 0){throw "Intrepid $Kind process exited $($capture.ExitCode); inspect $log"}
  if($overlap -and $Kind -eq 'profile'){throw 'Profile overlapped game/editor/shader/compiler work; do not use it as controlled performance evidence.'}
  $validationArguments=@($helper,'validate','--root',$root,'--folder',$driver,'--kind',$Kind,'--width',"$Width",'--height',"$Height",'--frames',"$Frames",'--started',$started.ToString('o'),'--log',$log,'--output',(Join-Path $controller 'validation.json'))
  if($Baseline){$validationArguments+='--baseline'};if($Rendered){$validationArguments+='--rendered'}
  if($Cabin){$validationArguments+='--cabin'};if($CabinBaseline){$validationArguments+='--cabin-baseline'};if($CabinReview){$validationArguments+='--cabin-review'}
  & $Python @validationArguments | Out-Null
  if($LASTEXITCODE -ne 0){throw 'Fresh driver checks/manifest/render artifacts failed semantic validation.'}
  if($Kind -eq 'profile'){
   $csvArguments=@($helper,'csv','--root',$root,'--folder',$driver,'--frames',"$Frames",'--fps-cap',"$FpsCap",'--output',(Join-Path $controller 'summary.json'))
   if($Cabin){$csvArguments+='--cabin'};if($CabinBaseline){$csvArguments+='--cabin-baseline'};if($Baseline){$csvArguments+='--baseline'}
   & $Python @csvArguments | Out-Null
   if($LASTEXITCODE -ne 0){throw 'Finite CSV frame count/counter validation failed.'}
  }
  $completed=$true
 } catch {$errorText=$_.Exception.Message} finally {
  if($capture -and -not $capture.HasExited){Stop-Process -Id $capture.Id -Force;$capture.WaitForExit();$environment.process_stopped_on_error=$true}
  $environment.finished_utc=[DateTime]::UtcNow.ToString('o');$environment.elapsed_wall_seconds=([DateTime]::UtcNow-$started).TotalSeconds
  if($capture -and $capture.HasExited){$environment.process_exit_code=$capture.ExitCode}
  try{
   & $Python $helper restore --root $root --manifest $snapshotManifest --output (Join-Path $controller 'preservation-result.json') | Out-Null
   if($LASTEXITCODE -ne 0){throw 'Original saved/config/photo/map preservation did not verify; inspect preservation-result.json.'}
  }catch{$restoreError=$_.Exception.Message;$completed=$false}
  try{
   $fingerprintAfter=Join-Path $controller 'fingerprint-after.json'
   $afterArguments=@($helper,'fingerprint','--root',$root,'--exe',$exe,'--output',$fingerprintAfter)
   if($module){$afterArguments+=@('--module',$module)}
   & $Python @afterArguments | Out-Null
   if($LASTEXITCODE -ne 0){throw 'Unable to fingerprint inputs after run.'}
   $after=Get-Content -LiteralPath $fingerprintAfter -Raw|ConvertFrom-Json
   if($before.input_sha256 -cne $after.input_sha256 -or $before.executable.sha256 -cne $after.executable.sha256 -or ($before.module -and $before.module.sha256 -cne $after.module.sha256)){throw 'Executable/module/source/art inputs changed during the run.'}
   $environment.input_fingerprints_unchanged=$true
  }catch{$completed=$false;if(-not $errorText){$errorText=$_.Exception.Message}}
  $environment.completed=$completed;$environment.error=$errorText;$environment.preservation_error=$restoreError
  $environment.competing_work_seen=$overlap;$environment.midrun_overlap_policy='Profile rejects any observed game/editor/compiler/shader work. Functional test/review records observed work without discarding the run; shader workers may belong to this UE process. Strict prelaunch idle guard applies to every kind.';$environment.maximum_sampled_working_set_bytes=$maximumWorkingSet;$environment.memory_sample_count=$samples.Count
  $environment.external_nvidia_sample_rows=$gpuSamples.Count;$environment.external_nvidia_command_available=[bool]$gpuCommand
  $environment | ConvertTo-Json -Depth 12|Set-Content -LiteralPath (Join-Path $controller 'environment.json') -Encoding utf8
  ConvertTo-Json -InputObject @($samples.ToArray()) -Depth 4|Set-Content -LiteralPath (Join-Path $controller 'memory-samples.json') -Encoding utf8
  @('timestamp,name,temperature_c,gpu_percent,graphics_mhz,memory_mhz,power_w')+@($gpuSamples.ToArray())|Set-Content -LiteralPath (Join-Path $controller 'gpu-samples.csv') -Encoding utf8
 }
 if(-not $completed){throw "Intrepid $Kind failed: $errorText $restoreError Evidence: $controller"}
 $validation=Get-Content -LiteralPath (Join-Path $controller 'validation.json') -Raw|ConvertFrom-Json
 Write-Output "PASS Intrepid $Kind ($($validation.assertions) assertions; baseline=$Baseline): $controller"
 if($Kind -eq 'profile'){
  $summary=Get-Content -LiteralPath (Join-Path $controller 'summary.json') -Raw|ConvertFrom-Json
  Write-Output "Captured $($summary.captured_frames) frames; mean FPS $($summary.mean_fps); p99 $($summary.p99_frame_ms) ms. Limited to this controlled four-camera run."
 }
}
