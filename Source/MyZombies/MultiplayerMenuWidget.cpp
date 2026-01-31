// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"


void UMultiplayerMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (MultiplayerBackButton)
	{
		MultiplayerBackButton->OnClicked.RemoveAll(this);
		MultiplayerBackButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::OnBackButtonClicked);
	}
}

void UMultiplayerMenuWidget::OnBackButtonClicked()
{
	// Returns to GameModeSelection if you arrived here via RequestPushMenu(...)
	RequestPopMenu();
}


