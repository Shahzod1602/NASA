#include "LunarApolloText.h"
#include "Internationalization/StringTableRegistry.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace {
bool Initialized=false;
bool Uzbek=true;
void Initialize(){
 if(Initialized)return;
 Initialized=true;
 LOCTABLE_NEW("Apollo12Uz","Apollo12Uz");
 LOCTABLE_NEW("Apollo12En","Apollo12En");
#define APOLLO_TEXT(Key, Uz, En) LOCTABLE_SETSTRING("Apollo12Uz",Key,Uz); LOCTABLE_SETSTRING("Apollo12En",Key,En);
#include "LunarApolloStrings.inl"
#undef APOLLO_TEXT
 FString Language;
 if(GConfig)GConfig->GetString(TEXT("LunarRescue.Accessibility"),TEXT("Language"),Language,GGameUserSettingsIni);
 FString Override;
 if(FParse::Value(FCommandLine::Get(),TEXT("ApolloLanguage="),Override))Language=Override;
 Uzbek=!Language.Equals(TEXT("en"),ESearchCase::IgnoreCase);
}
}

FText ApolloText::Text(const TCHAR* Key){Initialize();return FText::FromStringTable(Uzbek?TEXT("Apollo12Uz"):TEXT("Apollo12En"),Key);}
FString ApolloText::Get(const TCHAR* Key){return Text(Key).ToString();}
FString ApolloText::Format(const TCHAR* Key,const FFormatNamedArguments& Arguments){return FText::Format(Text(Key),Arguments).ToString();}
bool ApolloText::IsUzbek(){Initialize();return Uzbek;}
void ApolloText::SetUzbek(bool Value,bool SavePreference){
 Initialize();Uzbek=Value;
 const bool Automated=FParse::Param(FCommandLine::Get(),TEXT("SuitTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloTest"))||FParse::Param(FCommandLine::Get(),TEXT("LunarMenuTest"));
 if(SavePreference&&!Automated&&GConfig){
  GConfig->SetString(TEXT("LunarRescue.Accessibility"),TEXT("Language"),Uzbek?TEXT("uz"):TEXT("en"),GGameUserSettingsIni);
  GConfig->Flush(false,GGameUserSettingsIni);
 }
}
