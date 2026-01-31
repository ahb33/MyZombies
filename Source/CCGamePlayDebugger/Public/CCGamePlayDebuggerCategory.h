#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"
#endif

#if WITH_GAMEPLAY_DEBUGGER
class FCCGameplayDebuggerCategory final : public FGameplayDebuggerCategory
{
public:
    FCCGameplayDebuggerCategory() = default;

    static TSharedRef<FGameplayDebuggerCategory> MakeInstance()
    {
        return MakeShared<FCCGameplayDebuggerCategory>();
    }

    virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
    virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

private:
    TArray<FString> Lines;
};
#endif