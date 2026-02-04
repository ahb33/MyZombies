#include "FindSessionMenuWidget.h"
#include "MyGameInstance.h"
#include "JoinSessionMenuWidget.h"
#include "MyPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"


void UFindSessionMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureCachedRefs();
}

void UFindSessionMenuWidget::NativeDestruct()
{
    CachedSessionResults.Reset();

    if (SessionsScrollBox)
    {
        SessionsScrollBox->ClearChildren();
    }
    if (SessionsMessage)
    {
        SessionsMessage->SetText(FText::GetEmpty());
    }

    Super::NativeDestruct();
}

bool UFindSessionMenuWidget::SetupMultiplayerBinding()
{
	return EnsureCachedRefs();
}

bool UFindSessionMenuWidget::EnsureCachedRefs()
{
	if (!CachedGameInstance)
	{
		CachedGameInstance = Cast<UMyGameInstance>(GetGameInstance());
		if (!CachedGameInstance) return false;
	}

	if (!MultiplayerSessionRef)
	{
		MultiplayerSessionRef = CachedGameInstance->GetMultiplayerSessions();
		if (!MultiplayerSessionRef) return false;
	}

	if (!CachedMyPlayerController)
	{
		CachedMyPlayerController = Cast<AMyPlayerController>(GetOwningPlayer());
		if (!CachedMyPlayerController) return false;
	}
				
	return true;
}


void UFindSessionMenuWidget::AttemptFindSessions()
{
	if (!EnsureCachedRefs()) return;

	MultiplayerSessionRef->FindSessions(
		CachedMyPlayerController,
		10,
		GetSelectedGameModeCached().ToString()
	);
}

FName UFindSessionMenuWidget::GetSelectedGameModeCached() const
{
	return CachedGameInstance ? CachedGameInstance->GetSelectedGameMode() : NAME_None;
}

