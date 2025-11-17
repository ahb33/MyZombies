// Fill out your copyright notice in the Description page of Project Settings.


#include "YouDiedMenuWidget.h"
#include "Components/Button.h"
#include "MyPlayerController.h"




void UYouDiedMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayAgainButton)
	{
		PlayAgainButton->OnClicked.AddDynamic(this, &UYouDiedMenuWidget::OnPlayAgainClicked);
	}
	if (ExitToMainMenuButton)
	{
		ExitToMainMenuButton->OnClicked.AddDynamic(this, &UYouDiedMenuWidget::OnExitToMainMenuClicked);
	}
}

void UYouDiedMenuWidget::OnPlayAgainClicked()
{
	if (AMyPlayerController* PC = GetOwningPlayer<AMyPlayerController>())
	{
		PC->RequestRestartLevel(); // minimal: PC owns restart logic
	}
}

void UYouDiedMenuWidget::OnExitToMainMenuClicked()
{
	if (AMyPlayerController* PC = GetOwningPlayer<AMyPlayerController>())
	{
		// PC->GoToMainMenu(); // minimal: PC owns main menu travel
	}
}