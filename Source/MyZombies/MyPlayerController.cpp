// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "MainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "ReadyButtonWidget.h"
#include "YouDiedMenuWidget.h"
#include "ZombiesRoundWidget.h"
#include "CharacterStats.h"
#include "TimerManager.h"
#include "KillDeathStats.h"
#include "Components/AudioComponent.h"
#include "ZombiesGameState.h"
#include "Kismet/GameplayStatics.h"
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
    
    BindRoundDelegate();

}

void AMyPlayerController::BindRoundDelegate()
{
    CachedZGS = GetWorld() ? GetWorld()->GetGameState<AZombiesGameState>() : nullptr;
    
    if(!CachedZGS) return;

    CachedZGS->OnRoundStateChanged.RemoveAll(this);
    CachedZGS->OnRoundStateChanged.AddUObject(this, &AMyPlayerController::HandleRoundStateChanged);

    HandleRoundStateChanged(CachedZGS->GetRoundNumber(), CachedZGS->GetRoundPhase()); // initial sync
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CachedZGS) CachedZGS->OnRoundStateChanged.RemoveAll(this);

    Super::EndPlay(EndPlayReason);
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
    Client_ShowDeathScreen(); // server → owning client
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


void AMyPlayerController::ShowRoundIntroSplashWidget(int32 Round)
{
    // if(!RoundIntroWidgetClass) return;

    // if(!RoundIntroWidgetInstance)
    // {
    //     RoundIntroWidgetInstance = CreateWidget<URoundIntroSplashWidget>(this, RoundIntroWidgetClass);
    //     if(!RoundIntroWidgetInstance) return;
    // }

    // if(RoundIntroWidgetInstance && !RoundIntroWidgetInstance->IsInViewport())
    // {
    //     RoundIntroWidgetInstance->AddToViewport(1000);
    // }
    
    // RoundIntroWidgetInstance->SetVisibility(ESlateVisibility::Visible); 
    // RoundIntroWidgetInstance->SetRound(Round);
    // PlayRoundIntroSound(Round);



    // if (UWorld* World = GetWorld())
    // {
    //     World->GetTimerManager().ClearTimer(RoundIntroHideTimerHandle);
    //     World->GetTimerManager().SetTimer(RoundIntroHideTimerHandle, this, &AMyPlayerController::HideRoundIntroSplashWidget, RoundIntroWidgetDuration, false);
    // }
}

// void AMyPlayerController::HideRoundIntroSplashWidget()
// {
//     if (!RoundIntroWidgetInstance) return;

//     // why: reuse the same instance next round (no re-create, no duplicate widgets)
//     RoundIntroWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
// }


void AMyPlayerController::PlayRoundIntroSound(int32 Round)
{
    // UWorld* World = GetWorld();

    // if(!World) return;
    // if (RoundIntroThudComp && RoundIntroThudComp->IsPlaying()) RoundIntroThudComp->Stop();
    // if (RoundIntroVoiceComp && RoundIntroVoiceComp->IsPlaying()) RoundIntroVoiceComp->Stop();

    // float VoiceDelay = 0.15f;

    // if (RoundThudSound)
    // {
    //     RoundIntroThudComp = UGameplayStatics::SpawnSound2D(this, RoundThudSound);
    //     const float Dur = RoundThudSound->GetDuration();
    //     if (Dur > 0.f && Dur < 60.f) VoiceDelay = Dur;
    // }

    // const int32 Idx = Round - 1;

    // if (!RoundVoiceSounds.IsValidIndex(Idx) || !RoundVoiceSounds[Idx]) return;

    // auto& TM = World->GetTimerManager();
    // TM.ClearTimer(RoundVoiceTimerHandle);

    // TM.SetTimer(
    //     RoundVoiceTimerHandle,
    //     [WeakPC = TWeakObjectPtr<AMyPlayerController>(this), Voice = TWeakObjectPtr<USoundBase>(RoundVoiceSounds[Idx])]()
    //     {
    //         if (!WeakPC.IsValid() || !Voice.IsValid()) return;
    //         WeakPC->RoundIntroVoiceComp = UGameplayStatics::SpawnSound2D(WeakPC.Get(), Voice.Get());
    //     },
    //     VoiceDelay,
    //     false
    // );

}

void AMyPlayerController::InitRoundUI()
{
    if(!RoundHUDWidgetClass) return;

    if(!RoundHUDWidgetInstance)
    {
        RoundHUDWidgetInstance = CreateWidget<UZombiesRoundWidget>(this, RoundHUDWidgetClass);
        if(!RoundHUDWidgetInstance) return;
    }

    if(RoundHUDWidgetInstance && !RoundHUDWidgetInstance->IsInViewport())
    {
        RoundHUDWidgetInstance->AddToViewport(1000);
    }
    // This function should never change the round text; it only guarantees the widget exists.

}


void AMyPlayerController::UpdateZombiesRoundWidget(int32 Round)
{
    InitRoundUI();
    /*   
        Then call a method on the widget like SetRound(Round) (or set the TextBlock) to update the displayed “Round X”.
        This is what keeps the bottom corner always correct.      
    */
}



void AMyPlayerController::HandleRoundStateChanged(int32 RoundNumber, ERoundPhase Phase)
{
    
}

