// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "MyPlayerController.h"



// For a widget, NativeOnInitialized() is usually the cleanest place to bind once (instead of
void UPauseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (StartOverButton && !StartOverButton->OnClicked.IsAlreadyBound(this, &UPauseMenuWidget::OnStartOverButtonClicked))
    {
        StartOverButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnStartOverButtonClicked);
    }

    if (ResumeButton && !ResumeButton->OnClicked.IsAlreadyBound(this, &UPauseMenuWidget::OnResumeButtonClicked))
    {
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeButtonClicked);
    }

    if (QuitButton && !QuitButton->OnClicked.IsAlreadyBound(this, &UPauseMenuWidget::OnQuitButtonClicked))
    {
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitButtonClicked);
    }
}

void UPauseMenuWidget::OnStartOverButtonClicked()
{
	if (AMyPlayerController* PC = GetMyPC())
	{
		PC->RequestRestartLevel();
	}
}

void UPauseMenuWidget::OnResumeButtonClicked()
{
	if (AMyPlayerController* PC = GetMyPC())
	{
		PC->HidePauseMenu();
	}

}

void UPauseMenuWidget::OnQuitButtonClicked()
{
	if (AMyPlayerController* PC = GetMyPC())
	{
		PC->GoToMainMenu();
	}
}
AMyPlayerController* UPauseMenuWidget::GetMyPC() const
{
	return Cast<AMyPlayerController>(GetOwningPlayer());
}

void UPauseMenuWidget::ConfigureForMultiplayer(bool bIsMultiplayer)
{
    if (StartOverButton)
    {
        StartOverButton->SetVisibility(
            bIsMultiplayer ? ESlateVisibility::Collapsed: ESlateVisibility::Visible
        );
    }
}
