#include "LunarApolloText.h"
#include "Internationalization/StringTableRegistry.h"

namespace {
bool Initialized=false;
void Initialize(){
 if(Initialized)return;
 Initialized=true;
 LOCTABLE_NEW("Apollo12En","Apollo12En");
#define APOLLO_TEXT(Key, En) LOCTABLE_SETSTRING("Apollo12En",Key,En);
#include "LunarApolloStrings.inl"
#undef APOLLO_TEXT
}
}

FText ApolloText::Text(const TCHAR* Key){Initialize();return FText::FromStringTable(TEXT("Apollo12En"),Key);}
FString ApolloText::Get(const TCHAR* Key){return Text(Key).ToString();}
FString ApolloText::Format(const TCHAR* Key,const FFormatNamedArguments& Arguments){return FText::Format(Text(Key),Arguments).ToString();}
