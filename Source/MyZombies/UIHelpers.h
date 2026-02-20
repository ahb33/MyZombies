#pragma once

#include "CoreMinimal.h"


class AMyPlayerController;
class AActor;
class UUserWidget;


namespace UIHelpers
{
    
	/** Applies UI-only input mode, shows cursor, enables click/hover, and optionally focuses a widget. */
	void ApplyUIOnly(AMyPlayerController* PC, UUserWidget* FocusWidget = nullptr);

	/** Applies Game+UI input mode, shows cursor, enables click/hover, and optionally focuses a widget. */
	void ApplyGameAndUI(AMyPlayerController* PC, UUserWidget* FocusWidget = nullptr);

	/** Applies Game-only input mode and disables cursor/click/hover. */
	void ApplyGameOnly(AMyPlayerController* PC);

	/** Useful when you change replicated state and need it visible to clients immediately. */
	void ForceSafeNetUpdate(AActor* Actor);
}