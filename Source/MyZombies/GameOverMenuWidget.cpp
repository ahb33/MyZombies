// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOverMenuWidget.h"
#include "Components/Button.h"
#include "MyPlayerController.h"



void UGameOverMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayAgainButton)
	{
		PlayAgainButton->OnClicked.AddDynamic(this, &UGameOverMenuWidget::OnPlayAgainClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UGameOverMenuWidget::OnExitToMainMenuClicked);
	}
}

void UGameOverMenuWidget::OnPlayAgainClicked()
{
	if (AMyPlayerController* PC = GetOwningPlayer<AMyPlayerController>())
	{
		// PC->RequestRestartLevel(); // minimal: PC owns restart logic
	}
}

void UGameOverMenuWidget::OnExitToMainMenuClicked()
{
	if (AMyPlayerController* PC = GetOwningPlayer<AMyPlayerController>())
	{
		// PC->GoToMainMenu(); // minimal: PC owns main menu travel
	}
}