#include "CCGameplayDebugger.h"
#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"


#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "CCGameplayDebuggerCategory.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogCCGameplayDebugger, Log, All);


class FCCGameplayDebuggerModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
#if WITH_GAMEPLAY_DEBUGGER
		UE_LOG(LogCCGameplayDebugger, Warning, TEXT("CCGD StartupModule"));

        IGameplayDebugger& Debugger = IGameplayDebugger::Get();

 
		static const FName CategoryName(TEXT("CC"));
        const int32 Slot = 6;

        Debugger.UnregisterCategory(CategoryName);
        Debugger.RegisterCategory(
            CategoryName,
            IGameplayDebugger::FOnGetCategory::CreateStatic(&FCCGameplayDebuggerCategory::MakeInstance),
            EGameplayDebuggerCategoryState::EnabledInGameAndSimulate, Slot
        );

        Debugger.NotifyCategoriesChanged();
        UE_LOG(LogCCGameplayDebugger, Warning, TEXT("CCGD Registered category '%s' on slot %d"), *CategoryName.ToString(), Slot);
#endif
    }
    
    virtual void ShutdownModule() override
    {
#if WITH_GAMEPLAY_DEBUGGER
		if (!IGameplayDebugger::IsAvailable()) return;

		IGameplayDebugger& Debugger = IGameplayDebugger::Get();
		static const FName CategoryName(TEXT("CC"));

		Debugger.UnregisterCategory(CategoryName);
		Debugger.NotifyCategoriesChanged();
#endif
    }
};

IMPLEMENT_MODULE(FCCGameplayDebuggerModule, CCGameplayDebugger)