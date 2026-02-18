#include "UIHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

namespace UIHelpers
{
    
    void SetUIInputMode(AMyPlayerController* PC, bool bUIOnly, UWidget* FocusWidget)
    {
        if (!PC) return;

        if (bUIOnly)
        {
            FInputModeUIOnly Mode;
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            if (FocusWidget && FocusWidget->GetCachedWidget().IsValid())
            {
                Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
            }

            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = true;
            PC->bEnableClickEvents = true;
            PC->bEnableMouseOverEvents = true;
        }
        else
        {
            FInputModeGameOnly Mode;
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = false;
        }
    }

    void RestoreGameplayInput(AMyPlayerController* PC)
    {
        SetUIInputMode(PC, false);
    }

    void ForceSafeNetUpdate(AActor* Actor)
    {
        if (Actor && Actor->HasAuthority())
        {
            Actor->SetNetDormancy(DORM_Awake);
            Actor->FlushNetDormancy();
            Actor->ForceNetUpdate();
        }
    }
}