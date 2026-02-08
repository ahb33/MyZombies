// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeSelectionMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "BaseMenuWidget.h"
#include "MyGameInstance.h"


namespace
{
	static const FName MenuID_Multiplayer(TEXT("MultiplayerMenu"));
	static const FName GameMode_Deathmatch(TEXT("Deathmatch"));
	static const FName GameMode_Zombies(TEXT("Zombies_Level"));
}

void UGameModeSelectionMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

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

    if (BackButton && !BackButton->OnClicked.IsAlreadyBound(this, &UGameModeSelectionMenuWidget::OnBackButtonClicked))
    {
        BackButton->OnClicked.AddDynamic(this, &UGameModeSelectionMenuWidget::OnBackButtonClicked);
    }
}

UWidget* UGameModeSelectionMenuWidget::GetDefaultFocusWidget_Implementation() const
{
	// Pick the first button you want focused when menu opens
	if (DeathmatchButton) return DeathmatchButton;
	if (ZombiesButton)    return ZombiesButton;
	if (BackButton)       return BackButton;

	return Super::GetDefaultFocusWidget_Implementation();
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
	RequestPushMenu(MenuID_Multiplayer);
}


void UGameModeSelectionMenuWidget::OnBackButtonClicked()
{
	RequestPopMenu();
}


