// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerMenuWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"


void UMultiplayerMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (MultiplayerBackButton)
	{
		MultiplayerBackButton->OnClicked.RemoveAll(this);
		MultiplayerBackButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::OnBackButtonClicked);
	}

	SetBusy(false);
}

void UMultiplayerMenuWidget::NativeDestruct()
{
	if (MultiplayerBackButton)
	{
        MultiplayerBackButton->OnClicked.RemoveDynamic(this, &UMultiplayerMenuWidget::OnBackButtonClicked);
	}

	Super::NativeDestruct();
}

void UMultiplayerMenuWidget::OnBackButtonClicked()
{
	if (bIsBusy) return;
	RequestPopMenu();
}

void UMultiplayerMenuWidget::SetBusy(bool bBusy)
{
	bIsBusy = bBusy;
	if (ContentRoot)
	{
		ContentRoot->SetIsEnabled(!bBusy);
	}
	if (BusyBlocker)
	{
		BusyBlocker->SetVisibility(bBusy ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

}






