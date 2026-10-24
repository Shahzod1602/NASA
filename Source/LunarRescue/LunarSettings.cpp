#include "LunarSettings.h"
#include "LunarGame.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/IConsoleManager.h"

namespace {
const TMap<FName,FKey>& Defaults(){static const TMap<FName,FKey> Keys={
 {TEXT("MoveForward"),EKeys::W},{TEXT("MoveBack"),EKeys::S},{TEXT("MoveLeft"),EKeys::A},{TEXT("MoveRight"),EKeys::D},
 {TEXT("Interact"),EKeys::E},{TEXT("Jump"),EKeys::SpaceBar},{TEXT("Route"),EKeys::Q},{TEXT("Journal"),EKeys::Tab},
 {TEXT("View"),EKeys::V},{TEXT("Sprint"),EKeys::LeftShift},{TEXT("Zoom"),EKeys::F}};return Keys;}
}
bool ULunarSettings::IsAutomation(){
 const FString Args=FCommandLine::Get();
 return Args.Contains(TEXT("Test"))||Args.Contains(TEXT("Review"))||Args.Contains(TEXT("csvCapture"))||Args.Contains(TEXT("ApolloSaveProfile"));
}
ULunarSettings* ULunarSettings::Get(){
 static ULunarSettings* S=[](){auto* V=NewObject<ULunarSettings>();V->AddToRoot();if(IsAutomation())V->Reset();else V->LoadConfig();return V;}();return S;
}
FKey ULunarSettings::Key(FName Action) const {const FKey* K=Bindings.Find(Action);if(K&&K->IsValid())return *K;K=Defaults().Find(Action);return K?*K:EKeys::Invalid;}
bool ULunarSettings::Rebind(FName Action,FKey NewKey){
 if(!Defaults().Contains(Action)||!NewKey.IsValid()||NewKey.IsMouseButton()||NewKey==EKeys::Escape||NewKey==EKeys::F1||NewKey==EKeys::Enter||NewKey==EKeys::C||NewKey==EKeys::R||NewKey==EKeys::M||NewKey==EKeys::T||NewKey==EKeys::J||NewKey==EKeys::LeftAlt)return false;
 if(NewKey==EKeys::I||NewKey==EKeys::Up||NewKey==EKeys::Down||NewKey==EKeys::Left||NewKey==EKeys::Right)return false;
 for(const auto& Pair:Defaults())if(Pair.Key!=Action&&Key(Pair.Key)==NewKey)return false;
 Bindings.Add(Action,NewKey);return true;
}
void ULunarSettings::Reset(){Sensitivity=.65f;TextScale=SubtitleScale=1;EffectsVolume=RadioVolume=.8f;Brightness=1.1f;FieldOfView=85;InvertY=false;CameraMotion=true;ToggleSprint=false;Quality=1;Bindings.Reset();}
void ULunarSettings::Apply(ALunarCharacter* P,bool Save){
 Sensitivity=FMath::Clamp(Sensitivity,.15f,2.f);TextScale=FMath::Clamp(TextScale,1.f,1.5f);SubtitleScale=FMath::Clamp(SubtitleScale,1.f,1.5f);
 EffectsVolume=FMath::Clamp(EffectsVolume,0.f,1.f);RadioVolume=FMath::Clamp(RadioVolume,0.f,1.f);Brightness=FMath::Clamp(Brightness,.8f,1.4f);FieldOfView=FMath::Clamp(FieldOfView,70.f,100.f);Quality=FMath::Clamp(Quality,0,2);
 if(P){
  P->Camera->SetFieldOfView(FieldOfView);
  if(auto* PC=Cast<APlayerController>(P->GetController());PC&&PC->PlayerInput){
   auto* Input=PC->PlayerInput.Get();
   Input->ActionMappings=GetDefault<UInputSettings>()->GetActionMappings();Input->AxisMappings=GetDefault<UInputSettings>()->GetAxisMappings();
   for(auto& M:Input->ActionMappings)if(Defaults().Contains(M.ActionName))M.Key=Key(M.ActionName);
   for(auto& M:Input->AxisMappings){
    if(M.AxisName==TEXT("Forward")){if(M.Scale>0&&M.Key==EKeys::W)M.Key=Key(TEXT("MoveForward"));if(M.Scale<0&&M.Key==EKeys::S)M.Key=Key(TEXT("MoveBack"));}
    if(M.AxisName==TEXT("Right")){if(M.Scale>0&&M.Key==EKeys::D)M.Key=Key(TEXT("MoveRight"));if(M.Scale<0&&M.Key==EKeys::A)M.Key=Key(TEXT("MoveLeft"));}
   }
   Input->ForceRebuildingKeyMaps(false);
  }
 }
 if(GEngine)GEngine->DisplayGamma=2.2f*Brightness;
 int32 ProfileQuality=Quality;const bool Profile=FParse::Value(FCommandLine::Get(),TEXT("ApolloQuality="),ProfileQuality);
 if(!IsAutomation()||Profile)if(auto* GS=UGameUserSettings::GetGameUserSettings()){GS->SetOverallScalabilityLevel(Profile?FMath::Clamp(ProfileQuality,0,2):Quality);GS->SetFrameRateLimit(60);GS->ApplyNonResolutionSettings();if(Save&&!IsAutomation())GS->SaveSettings();}
 if(Save&&!IsAutomation())SaveConfig();
}
