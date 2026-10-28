#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputCoreTypes.h"
#include "LunarSettings.generated.h"

UCLASS(Config=GameUserSettings)
class LUNARRESCUE_API ULunarSettings : public UObject {
 GENERATED_BODY()
public:
 UPROPERTY(Config) float Sensitivity=.65f;
 UPROPERTY(Config) float TextScale=1.f;
 UPROPERTY(Config) float SubtitleScale=1.f;
 UPROPERTY(Config) float EffectsVolume=.8f;
 UPROPERTY(Config) float RadioVolume=.8f;
 UPROPERTY(Config) float Brightness=1.1f;
 UPROPERTY(Config) float FieldOfView=85.f;
 UPROPERTY(Config) bool InvertY=false;
 UPROPERTY(Config) bool CameraMotion=true;
 UPROPERTY(Config) bool ToggleSprint=false;
 UPROPERTY(Config) int32 Quality=1;
 UPROPERTY(Config) float MasterVolume=1.f;
 UPROPERTY(Config) float RenderScale=100.f;
 UPROPERTY(Config) int32 FrameLimit=60;
 UPROPERTY(Config) bool VSync=false;
 UPROPERTY(Config) int32 TextureQuality=-1;
 UPROPERTY(Config) int32 ShadowQuality=-1;
 UPROPERTY(Config) int32 ViewDistance=-1;
 UPROPERTY(Config) int32 AntiAliasing=2;
 UPROPERTY(Config) bool Bloom=true;
 UPROPERTY(Config) bool AmbientOcclusion=true;
 UPROPERTY(Config) bool MotionBlur=false;
 UPROPERTY(Config) bool Subtitles=true;
 UPROPERTY(Config) bool MouseSmoothing=false;
 // Transient: display changes must be confirmed, and never affect test profiles.
 bool DisplayPending=false;
 double DisplayDeadline=0;
 void PreviewDisplay(FIntPoint Resolution,int32 Mode);
 void ConfirmDisplay();
 void RevertDisplay();
 UPROPERTY(Config) TMap<FName,FKey> Bindings;
 static ULunarSettings* Get();
 static bool IsAutomation();
 FKey Key(FName Action) const;
 FString ControlText(FString Text) const;
 bool Rebind(FName Action,FKey NewKey);
 void Apply(class ALunarCharacter* Player,bool Save=true);
 void Reset();
};
