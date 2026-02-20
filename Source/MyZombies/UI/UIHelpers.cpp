#include "UIHelpers.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.h"
#include "Blueprint/UserWidget.h"

namespace UIHelpers
{
	static void SetMouseUIFlags(AMyPlayerController* PC, const bool bEnable)
	{
		if (!PC) return;
		PC->bShowMouseCursor = bEnable;
		PC->bEnableClickEvents = bEnable;
		PC->bEnableMouseOverEvents = bEnable;
	}

	static void FocusIfPossible(AMyPlayerController* PC, UUserWidget* FocusWidget)
	{
		if (!PC || !IsValid(FocusWidget)) return;
		FocusWidget->SetUserFocus(PC);
		FocusWidget->SetKeyboardFocus();
	}

	void ApplyUIOnly(AMyPlayerController* PC, UUserWidget* FocusWidget)
	{
		if (!PC) return;

		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		if (IsValid(FocusWidget))
		{
			Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
		}

		PC->SetInputMode(Mode);
		SetMouseUIFlags(PC, true);
		FocusIfPossible(PC, FocusWidget);
	}

	void ApplyGameAndUI(AMyPlayerController* PC, UUserWidget* FocusWidget)
	{
		if (!PC) return;

		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);

		if (IsValid(FocusWidget))
		{
			Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
		}

		PC->SetInputMode(Mode);
		SetMouseUIFlags(PC, true);
		FocusIfPossible(PC, FocusWidget);
	}

	void ApplyGameOnly(AMyPlayerController* PC)
	{
		if (!PC) return;

		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
		SetMouseUIFlags(PC, false);
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
