#include "CCDebug.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogCCDebug);

// cc.Debug 0/1 — toggle without rebuild
static TAutoConsoleVariable<int32> CVarCCDebug(
	TEXT("cc.Debug"), 1,
	TEXT("Enable CCDebug logs/timers. 0=off 1=on"),
	ECVF_Default);

class FCCDebugModule final : public IModuleInterface
{
public:
	void StartupModule() override { UE_LOG(LogCCDebug, Log, TEXT("CCDebug module started")); }
	void ShutdownModule() override { UE_LOG(LogCCDebug, Log, TEXT("CCDebug module shutdown")); }
};
IMPLEMENT_MODULE(FCCDebugModule, CCDebug)