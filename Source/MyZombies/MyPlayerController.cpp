// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "MainCharacter.h"
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

AMyPlayerController::AMyPlayerController() {}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	TryBindToGameState();

}


void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("StartGame", IE_Pressed, this, &AMyPlayerController::HandleReadyInput);
	}
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		World->GetTimerManager().ClearTimer(RoundVoiceTimerHandle);
		World->GetTimerManager().ClearTimer(RoundIntroHideTimerHandle);
	}

	if (RoundIntroThudComp && RoundIntroThudComp->IsPlaying()) RoundIntroThudComp->Stop();
	if (RoundIntroVoiceComp && RoundIntroVoiceComp->IsPlaying()) RoundIntroVoiceComp->Stop();

	UnbindFromGameState();

	Super::EndPlay(EndPlayReason);
}

void AMyPlayerController::TryBindToGameState()
{
	UnbindFromGameState();

	CachedBGS = GetWorld() ? GetWorld()->GetGameState<ABaseGameState>() : nullptr;
	if (!CachedBGS)
	{
		// Retry a few times in case GameState isn't ready yet on client.
		if (UWorld* World = GetWorld())
		{
			if (++BindRetryCount <= 20)
			{
				World->GetTimerManager().SetTimer(
					BindRetryTimerHandle,
					this,
					&AMyPlayerController::TryBindToGameState,
					0.2f,
					false
				);
			}
		}
		return;
	}

	BindRetryCount = 0;

	InputProfileChangedHandle =
    CachedBGS->OnInputProfileChanged.AddUObject(this, &AMyPlayerController::HandleInputProfileChanged);

	MatchPhaseChangedHandle =
		CachedBGS->OnMatchPhaseChanged.AddUObject(this, &AMyPlayerController::HandleMatchPhaseChanged);

	MatchModeChangedHandle =
		CachedBGS->OnMatchModeChanged.AddUObject(this, &AMyPlayerController::HandleMatchModeChanged);

	// Zombies-only GameState
	CachedZGS = GetWorld() ? GetWorld()->GetGameState<AZombiesGameState>() : nullptr;
	if (CachedZGS)
	{
		RoundNumberChangedHandle =
			CachedZGS->OnRoundNumberChanged.AddUObject(this, &AMyPlayerController::HandleRoundNumberChanged);
	}

	SyncFromCachedState();
}

void AMyPlayerController::UnbindFromGameState()
{
	if (CachedBGS)
	{
		if (InputProfileChangedHandle.IsValid()) CachedBGS->OnInputProfileChanged.Remove(InputProfileChangedHandle);
		if (MatchPhaseChangedHandle.IsValid()) CachedBGS->OnMatchPhaseChanged.Remove(MatchPhaseChangedHandle);
		if (MatchModeChangedHandle.IsValid()) CachedBGS->OnMatchModeChanged.Remove(MatchModeChangedHandle);
	}

	if (CachedZGS)
	{
		if (RoundNumberChangedHandle.IsValid()) CachedZGS->OnRoundNumberChanged.Remove(RoundNumberChangedHandle);
	}

	InputProfileChangedHandle.Reset();
	MatchPhaseChangedHandle.Reset();
	MatchModeChangedHandle.Reset();
	RoundNumberChangedHandle.Reset();

	CachedBGS = nullptr;
	CachedZGS = nullptr;
}

void AMyPlayerController::SyncFromCachedState()
{
	if (!CachedBGS) return;

	HandleMatchModeChanged(CachedBGS->GetMatchMode());
	HandleInputProfileChanged(CachedBGS->GetInputProfile());
	HandleMatchPhaseChanged(CachedBGS->GetMatchPhase());

	if (CachedZGS)
	{
		HandleRoundNumberChanged(CachedZGS->GetRoundNumber());
	}
}

void AMyPlayerController::HandleInputProfileChanged(EInputProfile Profile)
{
	ApplyInputProfile(Profile);

	// Lobby widget visibility is driven by InputProfile (not by map name).
	if (Profile == EInputProfile::Lobby)
	{
		if (ReadyButtonWidgetClass && !ReadyButtonWidgetInstance)
		{
			ReadyButtonWidgetInstance = CreateWidget<UReadyButtonWidget>(this, ReadyButtonWidgetClass);
		}
		if (ReadyButtonWidgetInstance && !ReadyButtonWidgetInstance->IsInViewport())
		{
			ReadyButtonWidgetInstance->AddToViewport();
		}
	}
	else
	{
		if (ReadyButtonWidgetInstance && ReadyButtonWidgetInstance->IsInViewport())
		{
			ReadyButtonWidgetInstance->RemoveFromParent();
		}
	}
}

void AMyPlayerController::HandleMatchModeChanged(EMatchMode Mode)
{
	// Zombies-only UI; hide if leaving Zombies.
	if (Mode != EMatchMode::Zombies)
	{
		if (RoundHUDWidgetInstance && RoundHUDWidgetInstance->IsInViewport())
		{
			RoundHUDWidgetInstance->RemoveFromParent();
		}
		if (RoundSplashWidgetInstance)
		{
			RoundSplashWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMyPlayerController::HandleMatchPhaseChanged(EMatchPhase Phase)
{
	switch (Phase)
	{
	case EMatchPhase::GameOver:
		ShowDeathScreenLocal();
		break;

	case EMatchPhase::Intro:
		// Splash only makes sense for Zombies.
		if (CachedBGS && CachedBGS->GetMatchMode() == EMatchMode::Zombies)
		{
			ShowRoundIntroSplashWidget(CachedRoundNumber);
		}
		break;

	default:
		// Leaving intro hides splash.
		HideRoundIntroSplashWidget();
		break;
	}
}

void AMyPlayerController::HandleRoundNumberChanged(int32 RoundNumber)
{
	CachedRoundNumber = FMath::Max(1, RoundNumber);

	if (!CachedBGS || CachedBGS->GetMatchMode() != EMatchMode::Zombies)
	{
		return;
	}

	EnsureRoundHUDWidget();
	if (RoundHUDWidgetInstance)
	{
		RoundHUDWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		RoundHUDWidgetInstance->SetRound(CachedRoundNumber);
	}

	// If the game is currently in Intro phase, show splash for the new round.
	if (CachedBGS->GetMatchPhase() == EMatchPhase::Intro)
	{
		ShowRoundIntroSplashWidget(CachedRoundNumber);
	}
}

void AMyPlayerController::ApplyInputProfile(EInputProfile Profile)
{
    switch(Profile)

    {
        case EInputProfile::Gameplay:
        {
            FInputModeGameOnly Mode;
            SetInputMode(Mode);

            bShowMouseCursor = false;
            bEnableClickEvents = false;
            bEnableMouseOverEvents = false;
            break;

        }

        case EInputProfile::Lobby:
        {
            UE_LOG(LogTemp, Warning, TEXT("Lobby input profile set"));
            FInputModeGameAndUI Mode;
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(Mode);

            bShowMouseCursor = true;
            bEnableClickEvents = true;
            bEnableMouseOverEvents = true;
            break;

        }
    }
}

void AMyPlayerController::HandleReadyInput()
{
	Server_SetPlayerReady();
}




void AMyPlayerController::TravelToLobby_Implementation()
{
    if(UWorld* World = GetWorld())
    {
        World->ServerTravel(TEXT("/Game/GameAssets/Levels/LobbyLevel?listen"));

    }
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
	AMyHUD* HUD = GetMyHUD();
	if (!HUD || !HUD->CharacterStats || !HUD->CharacterStats->HealthBar) return;

	const float HealthPercent = (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
	HUD->CharacterStats->HealthBar->SetPercent(HealthPercent);
}

void AMyPlayerController::SetHUDAmmo(int32 Ammo)
{
	AMyHUD* HUD = GetMyHUD();
	if (!HUD || !HUD->CharacterStats || !HUD->CharacterStats->AmmoOnDisplay) return;

	HUD->CharacterStats->AmmoOnDisplay->SetText(FText::AsNumber(Ammo));
}

void AMyPlayerController::SetHUDMagAmmo(int32 AmmoInMag)
{
	AMyHUD* HUD = GetMyHUD();
	if (!HUD || !HUD->CharacterStats || !HUD->CharacterStats->AmmoInMag) return;

	HUD->CharacterStats->AmmoInMag->SetText(FText::AsNumber(AmmoInMag));
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

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(DeathScreenInstance->TakeWidget());
		SetInputMode(Mode);

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
    }
}

void AMyPlayerController::EnsureRoundHUDWidget()
{
	if (!RoundHUDWidgetClass) return;

	if (!RoundHUDWidgetInstance)
	{
		RoundHUDWidgetInstance = CreateWidget<UZombiesRoundWidget>(this, RoundHUDWidgetClass);
	}
	if (RoundHUDWidgetInstance && !RoundHUDWidgetInstance->IsInViewport())
	{
		RoundHUDWidgetInstance->AddToViewport(1000);
	}
}

void AMyPlayerController::ShowRoundIntroSplashWidget(int32 RoundNumber)
{
	EnsureRoundSplashWidget();
	if (!RoundSplashWidgetInstance) return;

	if (!RoundSplashWidgetInstance->IsInViewport())
	{
		RoundSplashWidgetInstance->AddToViewport(1000);
	}

	RoundSplashWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	RoundSplashWidgetInstance->SetRound(RoundNumber);

	PlayRoundIntroSound(RoundNumber);

    RoundIntroWidgetDuration;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundIntroHideTimerHandle);
		World->GetTimerManager().SetTimer(
			RoundIntroHideTimerHandle,
			this,
			&AMyPlayerController::HideRoundIntroSplashWidget,
			RoundIntroWidgetDuration,
			false
		);
	}
}

void AMyPlayerController::EnsureRoundSplashWidget()
{
	if (!RoundSplashWidgetClass) return;

	if (!RoundSplashWidgetInstance)
	{
		RoundSplashWidgetInstance = CreateWidget<UZombiesRoundWidget>(this, RoundSplashWidgetClass);
	}
}



void AMyPlayerController::HideRoundIntroSplashWidget()
{
	if (RoundSplashWidgetInstance)
	{
		RoundSplashWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMyPlayerController::PlayRoundIntroSound(int32 RoundNumber)
{
	if (!GetWorld()) return;

	// Stop previous intro audio if still playing
	if (RoundIntroThudComp && RoundIntroThudComp->IsPlaying()) RoundIntroThudComp->Stop();
	if (RoundIntroVoiceComp && RoundIntroVoiceComp->IsPlaying()) RoundIntroVoiceComp->Stop();
	GetWorldTimerManager().ClearTimer(RoundVoiceTimerHandle);

	float VoiceDelay = 0.15f;

	if (RoundThudSound)
	{
		RoundIntroThudComp = UGameplayStatics::SpawnSound2D(this, RoundThudSound);
		VoiceDelay = FMath::Clamp(RoundThudSound->GetDuration(), 0.10f, 1.00f);
	}

	const int32 Index = RoundNumber - 1;
	if (RoundVoiceSounds.IsValidIndex(Index) && RoundVoiceSounds[Index])
	{
		USoundBase* VoiceSound = RoundVoiceSounds[Index];
		GetWorldTimerManager().SetTimer(
			RoundVoiceTimerHandle,
			FTimerDelegate::CreateUObject(this, &AMyPlayerController::PlayRoundVoiceSound, VoiceSound),
			VoiceDelay,
			false
		);
	}
}

void AMyPlayerController::PlayRoundVoiceSound(USoundBase* VoiceSound)
{
	if (!VoiceSound) return;
	RoundIntroVoiceComp = UGameplayStatics::SpawnSound2D(this, VoiceSound);
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


