#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"


class FCCGameplayDebuggerCategory final : public FGameplayDebuggerCategory
{
public:
    FCCGameplayDebuggerCategory();

    static TSharedRef<FGameplayDebuggerCategory> MakeInstance()
    {
        return MakeShared<FCCGameplayDebuggerCategory>();
    }

    virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
    virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

private:
	void ToggleRange();
	void ToggleOnlyAI();

	bool bDrawRange = true;
	bool bOnlyAI = true;
};
#endif