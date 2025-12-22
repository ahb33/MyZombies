// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "MainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "ReadyButtonWidget.h"
#include "YouDiedMenuWidget.h"
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
        ReadyButtonWidgetInstance = CreateWidget<UReadyButtonWidget>(this, ReadyButtonWidgetClass);
        if (ReadyButtonWidgetInstance)
        {
            ReadyButtonWidgetInstance->AddToViewport();
        }
    }
    
    else UE_LOG(LogTemp, Warning, TEXT("MapName invalid"));

}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if(HasAuthority())
    {
        if(auto* MC = Cast<AMainCharacter>(InPawn))
        {
            MC->OnMainCharacterDeath.RemoveAll(this); // avoid dup binds on re-possess
            MC->OnMainCharacterDeath.AddUniqueDynamic(this, &AMyPlayerController::HandlePlayerDeath);
        }
    }
}

void AMyPlayerController::OnUnPossess()
{
    if (HasAuthority())
        if (auto* MC = Cast<AMainCharacter>(GetPawn()))
            MC->OnMainCharacterDeath.RemoveDynamic(this, &AMyPlayerController::HandlePlayerDeath);
    Super::OnUnPossess();
}

void AMyPlayerController::HandlePlayerDeath()
{
    Client_ShowDeathScreen();               // server → owning client
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
        if (!MyPlayerHUD) 
        {
            UE_LOG(LogTemp, Warning, TEXT("MyPlayerHUD is not valid "));
            return;
        }
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


void AMyPlayerController::Client_ShowDeathScreen_Implementation()
{
    if (!IsLocalController()) return;
    ShowDeathScreenLocal(); 
}

void AMyPlayerController::ShowDeathScreenLocal()
{
    if (!DeathScreenClass) return;

    if (!DeathScreenInstance)
    {
        DeathScreenInstance = CreateWidget<UYouDiedMenuWidget>(this, DeathScreenClass);
    }
    if (DeathScreenInstance && !DeathScreenInstance->IsInViewport())
    {
        DeathScreenInstance->AddToViewport(1000);
        FInputModeUIOnly Mode; Mode.SetWidgetToFocus(DeathScreenInstance->TakeWidget());
        SetInputMode(Mode);
        bShowMouseCursor = true;
    }
}

void AMyPlayerController::RequestRestartLevel()
{
	// const FName LevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	// UGameplayStatics::OpenLevel(this, ZombieLevel);
}

void AMyPlayerController::GoToMainMenu()
{
	// static const FName MainMenuLevel(TEXT("MainMenu_Level"));
	// UGameplayStatics::OpenLevel(this, MainMenuLevel);
}

void AMyPlayerController::Client_PlayRoundIntro_Implementation(int32 Round)
{
    PlayRoundIntroSound(Round);
    ShowRoundIntroWidget(Round);

}

void AMyPlayerController::PlayRoundIntroSound(int32 Round)
{
}

void AMyPlayerController::ShowRoundIntroWidget(int32 Round)
{
}
