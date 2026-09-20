from pathlib import Path
import shutil
root=Path(__file__).resolve().parents[1]
if 'void ALunarGameMode::BuildStoryProps()' in (root/'Source/LunarRescue/LunarGame.cpp').read_text():
    raise SystemExit('Story source is already upgraded; edit LunarGame.cpp directly.')
backup=root/'Backups/BeforeStory'
backup.mkdir(parents=True,exist_ok=True)
for rel in ['Source/LunarRescue/LunarGame.h','Source/LunarRescue/LunarGame.cpp','Config/DefaultInput.ini']:
    if not (backup/Path(rel).name).exists(): shutil.copy2(root/rel,backup/Path(rel).name)
h=(root/'Source/LunarRescue/LunarGame.h').read_text()
h=h.replace(' void ZoomVisor();',' void ZoomVisor();\n void ReleaseInteract(); void ToggleRoute();\n UPROPERTY() TObjectPtr<class UStaticMeshComponent> CarriedModule;')
h=h.replace(' void PowerReview();',''' void PowerReview();
 void StoryReview();
 void BuildStoryProps();
 void ReleaseInteract();
 void ToggleRoute();
 void AdvanceInteraction(float Delta);
 float HoldDuration() const;
 int32 RepairStep=0;
 bool Holding=false, TrackRecorder=false, RecorderRecovered=false, LowOxygenWarned=false;
 float HoldProgress=0;
 UPROPERTY() TObjectPtr<AActor> RecorderActor;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> InstalledModule;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> PowerCable;''')
(root/'Source/LunarRescue/LunarGame.h').write_text(h)
p=root/'Source/LunarRescue/LunarGame.cpp'
s=p.read_text()
s=s.replace(' BuildGloves();',''' BuildGloves();
 CarriedModule=NewObject<UStaticMeshComponent>(this);
 CarriedModule->SetupAttachment(Camera);
 CarriedModule->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell")));
 CarriedModule->SetRelativeLocation(FVector(48,0,-31));CarriedModule->SetRelativeScale3D(FVector(.32));
 CarriedModule->SetCollisionEnabled(ECollisionEnabled::NoCollision);CarriedModule->SetCastShadow(false);
 CarriedModule->SetOnlyOwnerSee(true);CarriedModule->RegisterComponent();CarriedModule->SetVisibility(false);''')
s=s.replace(' // A few millimetres', ''' if(CarriedModule)CarriedModule->SetVisibility(G&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==0&&Camera->FieldOfView>60);
 // A few millimetres''')
s=s.replace('C->SetRelativeLocation(GloveRest[I]+FVector(0,0,Sway));','''const bool Carry=G&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==0;
  const float Reach=G&&G->Holding?FMath::Clamp(G->HoldProgress/.3f,0.f,1.f):0.f;
  C->SetRelativeLocation(GloveRest[I]+FVector(Reach*7,Carry?-GloveRest[I].Y*.15f:0,Sway+(Carry?4.f:Reach*3)));''')
s=s.replace(' I->BindAction("Interact",IE_Pressed,this,&ALunarCharacter::Interact);',''' I->BindAction("Interact",IE_Pressed,this,&ALunarCharacter::Interact);
 I->BindAction("Interact",IE_Released,this,&ALunarCharacter::ReleaseInteract);
 I->BindAction("Route",IE_Pressed,this,&ALunarCharacter::ToggleRoute);''')
s=s.replace('void ALunarCharacter::StartMission()', '''void ALunarCharacter::ReleaseInteract() {if(auto* G=Mission(this))G->ReleaseInteract();}
void ALunarCharacter::ToggleRoute() {if(auto* G=Mission(this))G->ToggleRoute();}
void ALunarCharacter::StartMission()''')
s=s.replace('G->Paused=!G->Paused;','G->Paused=!G->Paused; G->ReleaseInteract();')
s=s.replace(' Message("SIGNAL LOST. No response from SELENE research station.");',''' BuildStoryProps();
 Message("SIGNAL LOST. SELENE, do you copy? There should be someone here.");
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarStoryReview"))) {FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::StoryReview,8,false);}''')
s=s.replace('AdvanceTime(D); AdvancePowerSequence(D);','AdvanceTime(D); AdvancePowerSequence(D); AdvanceInteraction(D);')
s=s.replace('Message("POWER RESTORED. Recover the regolith temperature records from the terminal.");','Message("POWER RESTORED. Crew log: The rover did not fail. We shut it down.");')
s=s.replace(' Oxygen=FMath::Max(0.f,Oxygen-FMath::Max(0.f,D));',''' Oxygen=FMath::Max(0.f,Oxygen-FMath::Max(0.f,D));
 if(Oxygen>0&&Oxygen<60&&!LowOxygenWarned) {LowOxygenWarned=true;Message("CAUTION. One minute of oxygen. Return to the lander.");}''')
s=s.replace('Message("NAV: A spare power module is beside the eastern crater. Follow the marker.")','Message("NAV: Find the rover power cell. SELENE has stopped responding.")')
s=s.replace('case ELunarStage::ReturnHome:return HomeActor;', 'case ELunarStage::ReturnHome:return TrackRecorder&&!RecorderRecovered?RecorderActor.Get():HomeActor.Get();')
s=s.replace(' if(PowerSequenceActive)return "03 / STATION STARTING UP";',''' if(PowerSequenceActive)return "03 / STATION STARTING UP";
 if(Stage==ELunarStage::ReturnHome&&TrackRecorder&&!RecorderRecovered)return "OPTIONAL / RECOVER THE CREW RECORDER";''')
s=s.replace('case ELunarStage::RestorePower:return "[ E ]  Install power module";', 'case ELunarStage::RestorePower:return RepairStep==0?"[ E ]  Insert power cell":RepairStep==1?"[ E ]  Connect power cable":"[ HOLD E ]  Restart station";')
s=s.replace('case ELunarStage::CollectData:return "[ E ]  Download research data";', 'case ELunarStage::CollectData:return "[ HOLD E ]  Download science data";')
s=s.replace('case ELunarStage::ReturnHome:return "[ E ]  Complete mission";', 'case ELunarStage::ReturnHome:return TrackRecorder&&!RecorderRecovered?"[ HOLD E ]  Read crew recorder":"[ E ]  Complete mission";')
s=s.replace(' Q.AddIgnoredActor(P); Q.AddIgnoredActor(T);',' Q.AddIgnoredActor(P); Q.AddIgnoredActor(T); Q.AddIgnoredActor(this);')
s=s.replace(' FString Name=T.StartsWith', ' FString Name=T.StartsWith')
start=s.index(' FString Name=T.StartsWith')
end=s.index('\n if(!Name.IsEmpty())',start)
s=s[:start]+''' FString Name=T.StartsWith(TEXT("SIGNAL LOST"))?TEXT("StoryIntro"):T.StartsWith(TEXT("NAV:"))?TEXT("StoryStart"):T.StartsWith(TEXT("MODULE"))?TEXT("Module"):T.StartsWith(TEXT("POWER RESTORED"))?TEXT("StoryPower"):T.StartsWith(TEXT("DATA RECOVERED"))?TEXT("StoryData"):T.StartsWith(TEXT("CREW LOG"))?TEXT("StoryTruth"):T.StartsWith(TEXT("SIGNAL RESTORED"))?TEXT("StoryWin"):T.StartsWith(TEXT("CAUTION"))?TEXT("StoryWarning"):TEXT("");
 if(RadioAudio)RadioAudio->Stop();'''+s[end:]
start=s.index('void ALunarGameMode::Interact()')
end=s.index('\nvoid ALunarHUD::Label',start)
s=s[:start]+'''void ALunarGameMode::BuildStoryProps() {
 auto Make=[&](const TCHAR* Mesh,const TCHAR* Mat,FVector Pos,FVector Scale) {
  auto* C=NewObject<UStaticMeshComponent>(this);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Mesh));
  if(Mat)C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Mat));
  C->SetMobility(EComponentMobility::Movable);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  C->RegisterComponent();C->SetWorldLocation(Pos);C->SetWorldScale3D(Scale);return C;
 };
 if(PowerActor) {
  const FVector Port=PowerActor->GetActorLocation();
  InstalledModule=Make(TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell"),nullptr,Port+FVector(-69,0,-45),FVector(.5));
  InstalledModule->SetVisibility(false);
  PowerCable=Make(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),TEXT("/Game/Lunar/Materials/M_Warning.M_Warning"),Port+FVector(-59,40,0),FVector(.035,.035,1.1));
  PowerCable->SetVisibility(false);
 }
 // The optional recorder sits on a low survey ledge: a lunar jump reaches its top.
 FVector Site(5200,1900,0);FHitResult Ground;FCollisionQueryParams Q;Q.bTraceComplex=true;
 if(GetWorld()->LineTraceSingleByChannel(Ground,Site+FVector(0,0,2500),Site-FVector(0,0,1400),ECC_Visibility,Q))Site.Z=Ground.ImpactPoint.Z;
 auto* Ledge=GetWorld()->SpawnActor<AActor>();
 auto* Rock=NewObject<UStaticMeshComponent>(Ledge);Ledge->SetRootComponent(Rock);
 Rock->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 Rock->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Imported/Rocks/M_MoonRock01.M_MoonRock01")));
 Rock->SetCollisionProfileName(TEXT("BlockAll"));Rock->RegisterComponent();Ledge->SetActorLocation(Site+FVector(0,0,45));Rock->SetWorldScale3D(FVector(5,5,.9));
 RecorderActor=GetWorld()->SpawnActor<AActor>();auto* Recorder=NewObject<UStaticMeshComponent>(RecorderActor);RecorderActor->SetRootComponent(Recorder);
 Recorder->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Imported/ScienceTerminal/ScienceTerminal.ScienceTerminal")));
 Recorder->SetCollisionEnabled(ECollisionEnabled::NoCollision);Recorder->RegisterComponent();RecorderActor->SetActorLocation(Site+FVector(0,0,90));Recorder->SetWorldScale3D(FVector(.42));
 Make(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),TEXT("/Game/Lunar/Materials/M_Signal.M_Signal"),Site+FVector(110,0,165),FVector(.055,.055,1.5));
}
float ALunarGameMode::HoldDuration() const {
 if(Stage==ELunarStage::RestorePower&&RepairStep==2)return 2.f;
 if(Stage==ELunarStage::CollectData)return 3.f;
 if(Stage==ELunarStage::ReturnHome&&TrackRecorder&&!RecorderRecovered)return 2.f;
 return 0;
}
void ALunarGameMode::ReleaseInteract() {Holding=false;HoldProgress=0;}
void ALunarGameMode::ToggleRoute() {
 if(Stage!=ELunarStage::ReturnHome||Paused||RecorderRecovered)return;
 ReleaseInteract();TrackRecorder=!TrackRecorder;
 Message(TrackRecorder?"NAVIGATION: Crew recorder selected. Jump onto the survey ledge. Q returns to the lander route.":"NAVIGATION: Lander selected. The crew recorder is optional.");
}
void ALunarGameMode::AdvanceInteraction(float D) {
 if(Paused)return;
 if(!Holding)return;
 if(!CanInteract()||HoldDuration()<=0) {ReleaseInteract();return;}
 HoldProgress+=FMath::Max(0.f,D);
 if(HoldProgress<HoldDuration())return;
 ReleaseInteract();
 if(Stage==ELunarStage::RestorePower) {Stage=ELunarStage::CollectData;BeginPowerSequence();}
 else if(Stage==ELunarStage::CollectData) {Stage=ELunarStage::ReturnHome;Message("DATA RECOVERED. A crew recorder is on the survey ledge. Q selects the optional route.");}
 else if(Stage==ELunarStage::ReturnHome&&TrackRecorder) {
  RecorderRecovered=true;TrackRecorder=false;
  Message("CREW LOG: Cooling failed. We cut rover power to save the samples. Crew safe at the relay.");
 }
}
void ALunarGameMode::Interact() {
 if(!CanInteract()||Holding)return;
 if(HoldDuration()>0) {Holding=true;HoldProgress=0;return;}
 switch(Stage) {
 case ELunarStage::FindModule: ModuleActor->SetActorHiddenInGame(true);ModuleActor->SetActorEnableCollision(false);Stage=ELunarStage::RestorePower;Message("MODULE SECURED. Install it in the station's external power port.");break;
 case ELunarStage::RestorePower:
  if(RepairStep==0) {RepairStep=1;if(InstalledModule)InstalledModule->SetVisibility(true);Message("CELL SEATED. Connect the orange power cable.");}
  else if(RepairStep==1) {RepairStep=2;if(PowerCable)PowerCable->SetVisibility(true);Message("CABLE CONNECTED. Hold E to restart the station.");}
  if(auto* Click=LoadObject<USoundBase>(nullptr,TEXT("/Game/Lunar/Immersion/Audio/PowerContact.PowerContact")))UGameplayStatics::PlaySound2D(this,Click,.5f);
  break;
 case ELunarStage::ReturnHome:Stage=ELunarStage::Won;Message("SIGNAL RESTORED. We have your signal. Bring yourself home.");break;
 default:break;
 }
}
''' + s[end:]
s=s.replace('  auto* T=G->Target();','''  if(G->Stage==ELunarStage::RestorePower&&!G->PowerSequenceActive) {
   Panel(24*S,191*S,520*S,62*S,.83);
   Label(FString::Printf(TEXT("REPAIR %d / 3  -  %s"),G->RepairStep+1,G->RepairStep==0?TEXT("INSERT CELL"):G->RepairStep==1?TEXT("CONNECT CABLE"):TEXT("HOLD TO RESTART")),42*S,203*S,.46*S,Cyan);
   Label("Move close to the power port and look at it.",42*S,231*S,.37*S,Muted);
  }
  if(G->Stage==ELunarStage::ReturnHome) {
   Panel(24*S,191*S,570*S,75*S,.83);
   Label(G->RecorderRecovered?"CREW LOG RECOVERED / CREW SAFE":G->TrackRecorder?"[ Q ]  SWITCH BACK TO LANDER":"[ Q ]  OPTIONAL: INVESTIGATE CREW RECORDER",42*S,203*S,.43*S,Cyan);
   if(!G->RecorderRecovered)Label("Extra travel uses oxygen. Returning now still completes the mission.",42*S,233*S,.35*S,Muted);
  }
  if(G->Holding) {
   const float Progress=FMath::Clamp(G->HoldProgress/G->HoldDuration(),0.f,1.f);
   Panel(W/2-220*S,H-243*S,440*S,28*S,.9);
   DrawRect(Cyan,W/2-210*S,H-234*S,420*S*Progress,8*S);
  }
  auto* T=G->Target();''')
s=s.replace('F  Visor zoom     ESC  Pause','F  Zoom     Q  Route     ESC  Pause')
s=s.replace('Tip: move close to an object, look at it, then press E.','Tip: hold E during restart and data download.')
s=s.replace('G->Stage==ELunarStage::Won?"MISSION COMPLETE"', 'G->Stage==ELunarStage::Won?(G->RecorderRecovered?"THE TRUTH RECOVERED":"MISSION COMPLETE")')
s=s.replace('"The station is online. Research data has been delivered safely."','"We have your signal. Bring yourself home."')
s=s.replace('Label(G->Stage==ELunarStage::Won?"Science goal: study extreme surface temperature changes":"Tip: hold E during restart and data download.",X+36*S,Y+195*S,.44*S,Muted);','''Label(G->Stage==ELunarStage::Won?FString::Printf(TEXT("O2 REMAINING  %02d:%02d   /   POWER ONLINE   /   SCIENCE SECURED"),FMath::CeilToInt(G->Oxygen)/60,FMath::CeilToInt(G->Oxygen)%60):FString("Tip: hold E during restart and data download."),X+36*S,Y+195*S,.42*S,Muted);''')
s=s.replace('Label("to improve thermal protection for future lunar equipment.",X+36*S,Y+224*S,.44*S,Muted);','Label(G->RecorderRecovered?"LOGS 2/2 - Crew evacuated safely. Samples saved by emergency shutdown.":"LOGS 1/2 - The crew recorder remains on the survey ledge.",X+36*S,Y+224*S,.4*S,Muted);')
# Adapt the existing integration test to multi-step interactions.
s=s.replace('Check(CanInteract(),*FString::Printf(TEXT("Target %d is reachable and visible"),I)); Interact();','''Check(CanInteract(),*FString::Printf(TEXT("Target %d is reachable and visible"),I)); Interact();
   if(I==0)Check(ModuleActor->IsHidden()&&RepairStep==0,TEXT("Power cell picked up without enabling station"));
   if(I==1) {
    Check(RepairStep==1&&InstalledModule&&InstalledModule->IsVisible(),TEXT("Power cell visibly inserted"));
    Interact();Check(RepairStep==2&&PowerCable&&PowerCable->IsVisible(),TEXT("Cable connected as separate repair step"));
    Interact();AdvanceInteraction(.8f);ReleaseInteract();Check(HoldProgress==0&&!PowerSequenceActive,TEXT("Released hold resets restart progress"));
    Interact();Paused=true;AdvanceInteraction(5);Check(HoldProgress==0,TEXT("Paused hold does not advance"));Paused=false;
    AdvanceInteraction(2.1f);
   }
   if(I==2) {
    AdvanceInteraction(1);PC->SetControlRotation(FRotator(0,180,0));AdvanceInteraction(1);Check(!Holding&&Stage==ELunarStage::CollectData,TEXT("Looking away cancels download"));
    PC->SetControlRotation((T->GetActorLocation()-Eye).Rotation());Interact();AdvanceInteraction(3.1f);
   }''')
s=s.replace('  float Saved=Oxygen;', '''  Check(!RecorderRecovered&&Stage==ELunarStage::Won,TEXT("Optional recorder may be skipped for normal victory"));
  Stage=ELunarStage::ReturnHome;ToggleRoute();Check(Target()==RecorderActor,TEXT("Q selects optional recorder"));
  if(RecorderActor) {
   P->SetActorLocation(RecorderActor->GetActorLocation()+FVector(-100,0,100),false,nullptr,ETeleportType::TeleportPhysics);
   FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((RecorderActor->GetActorLocation()-Eye).Rotation());
   Check(CanInteract(),TEXT("Recorder reachable from survey ledge"));Interact();AdvanceInteraction(2.1f);
   Check(RecorderRecovered&&!TrackRecorder&&Target()==HomeActor,TEXT("Recorder reveals crew fate and restores home marker"));
   ToggleRoute();Check(!TrackRecorder,TEXT("Recovered recorder cannot be collected twice"));
   P->SetActorLocation(HomeActor->GetActorLocation()+FVector(-180,0,40),false,nullptr,ETeleportType::TeleportPhysics);
   P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((HomeActor->GetActorLocation()-Eye).Rotation());Interact();
   Check(Stage==ELunarStage::Won&&RecorderRecovered,TEXT("Full story ending completes at lander"));
  }
  float Saved=Oxygen;''')
s+='''
void ALunarGameMode::StoryReview() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 Oxygen=240;
 if(MovementPhase==0) {Stage=ELunarStage::RestorePower;RepairStep=0;P->SetActorLocation(FVector(-430,-1000,100));PC->SetControlRotation(FRotator(-10,0,0));PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true);}
 if(MovementPhase==1)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-carry.png"),true,false);
 if(MovementPhase==2) {RepairStep=2;InstalledModule->SetVisibility(true);PowerCable->SetVisibility(true);}
 if(MovementPhase==3)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-repair.png"),true,false);
 if(MovementPhase==4) {Stage=ELunarStage::ReturnHome;TrackRecorder=true;P->SetActorLocation(RecorderActor->GetActorLocation()+FVector(-550,0,180));PC->SetControlRotation(FRotator(-10,0,0));Message("DATA RECOVERED. A crew recorder is on the survey ledge. Q selects the optional route.");}
 if(MovementPhase==5)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-recorder.png"),true,false);
 if(MovementPhase==6) {Stage=ELunarStage::Won;RecorderRecovered=true;}
 if(MovementPhase==7)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-ending.png"),true,false);
 if(MovementPhase==8){FPlatformMisc::RequestExitWithStatus(false,0);return;}
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::StoryReview,3,false);
}
'''
p.write_text(s)
f=root/'Config/DefaultInput.ini'
with f.open('a') as out:out.write('\n+ActionMappings=(ActionName="Route",Key=Q)\n')
print('Story source upgrade written')
