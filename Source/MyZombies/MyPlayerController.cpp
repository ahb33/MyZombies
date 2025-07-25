// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "LobbyMenuWidget.h"
#include "Net/UnrealNetwork.h"
#include "WBP_ReadyButtonWidget.h"
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

    InputComponent->BindAction("ReadyUp", IE_Pressed, this, &AMyPlayerController::HandleReadyInput);
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
    if (!MyPlayerHUD)
    {
        MyPlayerHUD = Cast<AMyHUD>(GetHUD());
        if (!MyPlayerHUD) return;
    }

    if (MyPlayerHUD->CharacterStats && MyPlayerHUD->CharacterStats->AmmoOnDisplay)
    {
        MyPlayerHUD->CharacterStats->AmmoOnDisplay->SetText(FText::AsNumber(Ammo));
    }
}

void AMyPlayerController::SetHUDMagAmmo(int32 AmmoInMag)
{
    if (!MyPlayerHUD)
    {
        MyPlayerHUD = Cast<AMyHUD>(GetHUD());
        if (!MyPlayerHUD) return;
    }

    if (MyPlayerHUD->CharacterStats && MyPlayerHUD->CharacterStats->AmmoInMag)
    {
        MyPlayerHUD->CharacterStats->AmmoInMag->SetText(FText::AsNumber(AmmoInMag));
    }
}