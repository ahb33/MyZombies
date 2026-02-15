#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

// CustomCategory class
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
    void ToggleVision();

	bool bDrawRange = true;
	bool bOnlyAI = true;
    bool bDrawVision = true;

    
};
#endif