#include "Modules/ModuleManager.h"
#include "LunarLoading.h"
class FLunarRescueModule : public FDefaultGameModuleImpl {
public:
 virtual void StartupModule() override {LunarLoading::Initialize();}
 virtual void ShutdownModule() override {LunarLoading::Shutdown();}
};
IMPLEMENT_PRIMARY_GAME_MODULE(FLunarRescueModule, LunarRescue, "LunarRescue");
