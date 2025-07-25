// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerMenuWidget.h"
#include "Kismet/GameplayStatics.h"


void UMultiplayerMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    MenuSetup();
}

void UMultiplayerMenuWidget::MenuSetup()
{
    Super::MenuSetup();
    BindButtonEvents(); 

}


void UMultiplayerMenuWidget::BindButtonEvents()
{
    if (MultiplayerBackButton && !MultiplayerBackButton->OnClicked.IsAlreadyBound(this, &UMultiplayerMenuWidget::OnBackButtonClicked))
    MultiplayerBackButton->OnClicked.AddDynamic(this, &UMultiplayerMenuWidget::OnBackButtonClicked);
}


void UMultiplayerMenuWidget::OnBackButtonClicked()
{
    // Ensure widget creation before attempting transition
    UE_LOG(LogTemp, Warning, TEXT("Back button clicked in MultiplayerMenuWidget"));

    FName GameModeSelectionMenu = "GameModeMenu";

    auto gameModeMenuWidgetRef = GetGameModeSelectionMenuWidgetClass();

    if (!menuWidgetMap.Contains(GameModeSelectionMenu))
    {
        Super::CreateAndStoreWidget(GameModeSelectionMenu, gameModeMenuWidgetRef);
    }

    TransitionToMenu(GameModeSelectionMenu);
}


