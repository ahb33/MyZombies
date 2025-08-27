// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "LobbyMenuWidget.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "WBP_ReadyButtonWidget.h"
#include "CharacterStats.h"
#include "KillDeathStats.h"
#include "Blueprint/UserWidget.h"

AMyPlayerController::AMyPlayerController() {}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController()) return;

    const FString MapName = GetWorld()->GetMapName();
    if (MapName.Contains("LobbyLevel") && ReadyButtonWidgetClass)
    {
        ReadyButtonWidgetInstance = CreateWidget<UWBP_ReadyButtonWidget>(this, ReadyButtonWidgetClass);
        if (ReadyButtonWidgetInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("Adding Ready Button to viewport"));
            ReadyButtonWidgetInstance->AddToViewport();
        }
        else UE_LOG(LogTemp, Warning, TEXT("ReadyButtonWidgetInstance invalid"));
    }
    
    else UE_LOG(LogTemp, Warning, TEXT("MapName invalid"));

}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("StartGame", IE_Pressed, this, &AMyPlayerController::HandleReadyInput);
}

void AMyPlayerController::HandleReadyInput()
{
    Server_SetPlayerReady();
}

void AMyPlayerController::TravelToLobby_Implementation()
{
    GetWorld()->ServerTravel(TEXT("/Game/GameAssets/Levels/LobbyLevel?listen"));
}

void AMyPlayerController::Server_SetPlayerReady_Implementation()
{
    if (ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>())
    {
        PS->SetReadyStatus(true);
    }
}

void AMyPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
    if (!MyPlayerHUD)
    {
        MyPlayerHUD = Cast<AMyHUD>(GetHUD());
        if (!MyPlayerHUD) return;
    }

    if (MyPlayerHUD->CharacterStats && MyPlayerHUD->CharacterStats->HealthBar)
    {
        const float HealthPercent = CurrentHealth / MaxHealth;
        MyPlayerHUD->CharacterStats->HealthBar->SetPercent(HealthPercent);
    }
}

void AMyPlayerController::SetHUDAmmo(int32 Ammo)
{
    if (AMyHUD* HUD = GetMyHUD())
    {
        if (HUD->CharacterStats && HUD->CharacterStats->AmmoOnDisplay)
        {
            HUD->CharacterStats->AmmoOnDisplay->SetText(FText::AsNumber(Ammo));
        }
    }
}

void AMyPlayerController::SetHUDMagAmmo(int32 AmmoInMag)
{
    if (AMyHUD* HUD = GetMyHUD())
    {
        if (HUD->CharacterStats && HUD->CharacterStats->AmmoInMag)
        {
            HUD->CharacterStats->AmmoInMag->SetText(FText::AsNumber(AmmoInMag));
        }
    }
}

void AMyPlayerController::UpdateHUDKillDeath(int32 Kills, int32 Deaths)
{
    if (AMyHUD* HUD = GetMyHUD())
    {
        if (HUD->KillDeathStats && HUD->KillDeathStats->PlayerKills && HUD->KillDeathStats->PlayerDeaths)
        {
            HUD->KillDeathStats->PlayerKills->SetText(FText::AsNumber(Kills));
            HUD->KillDeathStats->PlayerDeaths->SetText(FText::AsNumber(Deaths));
        }
    }
}

AMyHUD* AMyPlayerController::GetMyHUD()
{
    if (!MyPlayerHUD)
    {
        MyPlayerHUD = Cast<AMyHUD>(GetHUD());
    }
    return MyPlayerHUD;
}
