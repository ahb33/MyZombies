// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeSelectionMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h"


void UGameModeSelectionMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MenuSetup();

}

void UGameModeSelectionMenuWidget::MenuSetup()
{
    Super::MenuSetup();
    BindButtonEvents(); 


}

void UGameModeSelectionMenuWidget::BindButtonEvents()
{
    if (DeathmatchButton && !DeathmatchButton->OnClicked.IsAlreadyBound(this, &UGameModeSelectionMenuWidget::OnSelectDeathmatchButtonClicked))
    {
        DeathmatchButton->OnClicked.AddDynamic(this, &UGameModeSelectionMenuWidget::OnSelectDeathmatchButtonClicked);
    }

    if (ZombiesButton && !ZombiesButton->OnClicked.IsAlreadyBound(this, &UGameModeSelectionMenuWidget::OnSelectZombiesButtonClicked))
    {
        ZombiesButton->OnClicked.AddDynamic(this, &UGameModeSelectionMenuWidget::OnSelectZombiesButtonClicked);
    }

    if (BackButton && !BackButton->OnClicked.IsAlreadyBound(this, &UGameModeSelectionMenuWidget::OnBackToMainMenuButtonClicked))
    {
        BackButton->OnClicked.AddDynamic(this, &UGameModeSelectionMenuWidget::OnBackToMainMenuButtonClicked);
    }
}


void UGameModeSelectionMenuWidget::OnSelectDeathmatchButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Deathmatch Button Clicked"));
    HandleGameModeSelection("Deathmatch");
}

void UGameModeSelectionMenuWidget::OnSelectZombiesButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Zombies Button Clicked"));
    HandleGameModeSelection("Zombies_Level");
}

void UGameModeSelectionMenuWidget::HandleGameModeSelection(FName GameModeName)
{
    if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting GameInstance mode to: %s"), *GameModeName.ToString());
        GI->SetSelectedGameMode(GameModeName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to cast GameInstance"));
    }

    FName MultiplayerMenu = "MultiplayerMenu";
    auto MultiplayerMenuWidgetRef = GetMultiplayerMenuWidgetClass();

    if (!menuWidgetMap.Contains(MultiplayerMenu))
    {
        UE_LOG(LogTemp, Log, TEXT("Creating MultiplayerMenuWidget widget."));
        Super::CreateAndStoreWidget(MultiplayerMenu, MultiplayerMenuWidgetRef);
    }

    TransitionToMenu(MultiplayerMenu);
}


void UGameModeSelectionMenuWidget::OnBackToMainMenuButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("BackButtonClicked"));


    FName MainMenu = "MainMenu";

    auto mainMenuWidgetRef = GetMainMenuWidgetClass();

    if (!menuWidgetMap.Contains(MainMenu))
    {
        UE_LOG(LogTemp, Log, TEXT("Creating MainMenu widget."));
        Super::CreateAndStoreWidget(MainMenu, mainMenuWidgetRef);
    }

    TransitionToMenu(MainMenu);
}


