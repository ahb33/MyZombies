
#include "MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "MyPlayerController.h"
#include "MultiplayerMenuWidget.h"


void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SoloButton)
	{
		SoloButton->OnClicked.RemoveAll(this);
		SoloButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSoloClicked);
	}

	if (MultiplayerButton)
	{
		MultiplayerButton->OnClicked.RemoveAll(this);
		MultiplayerButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnMultiplayerClicked);
	}
}

void UMainMenuWidget::OnSoloClicked()
{
	// Push so Solo menu can Pop() back to Main
	RequestPushMenu(TEXT("SoloMenu"));
}

void UMainMenuWidget::OnMultiplayerClicked()
{
	// Push so selection menu can Pop() back to Main
	RequestPushMenu(TEXT("GameModeSelectionMenu"));
}

UWidget* UMainMenuWidget::GetDefaultFocusWidget_Implementation() const
{
    if (SoloButton) return SoloButton;
    if (MultiplayerButton) return MultiplayerButton;
    return Super::GetDefaultFocusWidget_Implementation();
}
