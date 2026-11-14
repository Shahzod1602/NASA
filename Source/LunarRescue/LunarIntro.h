#pragma once
#include "CoreMinimal.h"

// Authored UI timing, independent of mission resources. A new world does not
// replay the process-start intro; the menu owns that single-launch decision.
struct FLunarIntroState {
 static constexpr float FadeIn=0.f,Hold=1.1f,FadeOut=.65f,Duration=1.75f;
 float Elapsed=0;
 bool Active=false;
 void Begin(){Elapsed=0;Active=true;}
 void Advance(float Delta){
  if(!Active||!FMath::IsFinite(Delta)||Delta<=0)return;
  Elapsed=FMath::Min(Duration,Elapsed+Delta);
  if(Elapsed>=Duration-UE_KINDA_SMALL_NUMBER){Elapsed=Duration;Active=false;}
 }
 static float Ease(float T){if(T<=0)return 0;if(T>=1)return 1;return T*T*(3-2*T);}
 float ImageAlpha() const{return 1.f;}
 float LayerAlpha() const{return Elapsed>=Duration?0.f:1-Ease((Elapsed-FadeIn-Hold)/FadeOut);}
 bool RevealingMenu() const{return !Active||Elapsed>=FadeIn+Hold;}
};
