#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "CCGameplayDebuggerCategory.h"
#endif


class FCCGameplayDebuggerModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
#if WITH_GAMEPLAY_DEBUGGER
        if (!IGameplayDebugger::IsAvailable())
        {
            return;
        }

        IGameplayDebugger& Debugger = IGameplayDebugger::Get();

        static const FName CategoryName(TEXT("CC"));
        Debugger.RegisterCategory(
            CategoryName,
            IGameplayDebugger::FOnGetCategory::CreateStatic(&FCCGameplayDebuggerCategory::MakeInstance),
            EGameplayDebuggerCategoryState::EnabledInGameAndSimulate,
            /*SlotIdx=*/5
        );

        Debugger.NotifyCategoriesChanged();
#endif
    }
    
    virtual void ShutdownModule() override
    {
#if WITH_GAMEPLAY_DEBUGGER
        if (IGameplayDebugger::IsAvailable())
        {
            IGameplayDebugger::Get().UnregisterCategory(TEXT("CC"));
            IGameplayDebugger::Get().NotifyCategoriesChanged();
        }
#endif
    }
};

IMPLEMENT_MODULE(FCCGameplayDebuggerModule, CCGameplayDebugger)