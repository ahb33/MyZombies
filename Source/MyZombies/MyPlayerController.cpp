// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ReadyButtonWidget.h"
#include "GameOverMenuWidget.h"
#include "ZombiesRoundWidget.h"
#include "Components/ProgressBar.h" 
#include "PauseMenuWidget.h"
#include "CharacterStats.h"
#include "BaseGameState.h"
#include "MenuUIManager.h"
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

    const FString Map = UGameplayStatics::GetCurrentLevelName(this, true);

    // Only build/show menu UI on the menu map
    if (Map == TEXT("MainMenu_Level")) 
    {
        MenuUI = NewObject<UMenuUIManager>(this);
        MenuUI->Init(this);

        MenuUI->RegisterMenu("MainMenu", MainMenuWidgetClass, 0);
        MenuUI->RegisterMenu("SoloMenu", SoloMenuWidgetClass, 0);
        MenuUI->RegisterMenu("GameModeSelectionMenu", GameModeSelectionMenuWidgetClass, 0);
        MenuUI->RegisterMenu("MultiplayerMenu", MultiplayerMenuWidgetClass, 0); 
        MenuUI->RegisterMenu("CreateSessionMenu", CreateSessionMenuWidgetClass, 0);
        MenuUI->RegisterMenu("JoinSessionMenu", JoinSessionMenuWidgetClass, 0);

        MenuUI->ShowMenu("MainMenu");
        ApplyInputProfile(EInputProfile::Menu);
    }
    else
    {
        ApplyInputProfile(EInputProfile::Gameplay);
        // optional: if MenuUI exists, hide/clear it here
    }
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("StartGame", IE_Pressed, this, &AMyPlayerController::HandleReadyInput);
		auto& Binding  = InputComponent->BindAction("Pause", IE_Pressed, this, &AMyPlayerController::TogglePauseMenu);
		Binding.bExecuteWhenPaused = true;
	}
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        auto& TM = World->GetTimerManager();
        TM.ClearTimer(BindRetryTimerHandle);
        TM.ClearTimer(RoundVoiceTimerHandle);
        TM.ClearTimer(RoundIntroHideTimerHandle);
	}

	if (RoundIntroThudComp && RoundIntroThudComp->IsPlaying()) RoundIntroThudComp->Stop();
	if (RoundIntroVoiceComp && RoundIntroVoiceComp->IsPlaying()) RoundIntroVoiceComp->Stop();

	UnbindFromGameState();

	Super::EndPlay(EndPlayReason);
}

void AMyPlayerController::BeginPlayingState()
{
    Super::BeginPlayingState();
    UpdateUIForCurrentMap();
}

void AMyPlayerController::TryBindToGameState()
{
	UnbindFromGameState();

	UWorld* World = GetWorld();

	CachedBGS = World->GetGameState<ABaseGameState>();
    if (!CachedBGS)
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
    const FString Map = UGameplayStatics::GetCurrentLevelName(this, true);
    const bool bIsMainMenu = (Map == TEXT("MainMenu_Level"));
    const bool bIsLobby    = (Map == TEXT("LobbyLevel"));

    if (bIsMainMenu || bIsLobby)
    {
        ApplyInputProfile(EInputProfile::Menu);
    }
    else
    {
        ApplyInputProfile(Profile); // gameplay maps only
    }

    // ReadyButton is ONLY lobby-level UI (not input-profile UI)
    EnsureReadyButton();
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
		// when gamemode switches from into to active
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

	// // If the game is currently in Intro phase, show splash for the new round.
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
			break;
		}

        case EInputProfile::Menu:
		{
            FInputModeGameAndUI Mode;
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Mode.SetHideCursorDuringCapture(false);

            SetInputMode(Mode);

            bShowMouseCursor = true;
            bEnableClickEvents = true;
            bEnableMouseOverEvents = true;
            break;
		}
		default:
			break;			
    }
}


void AMyPlayerController::HandleReadyInput()
{
	if(bReadyRequestInFlight) return;
	bReadyRequestInFlight = true;

	if (ReadyButtonWidgetInstance)
    {
        ReadyButtonWidgetInstance->SetReadyPending(true);
    }

	Server_SetPlayerReady();
}

void AMyPlayerController::UpdateUIForCurrentMap()
{
    const FString Map = UGameplayStatics::GetCurrentLevelName(this, true);

    const bool bIsMainMenu = (Map == TEXT("MainMenu_Level"));
    const bool bIsLobby    = (Map == TEXT("LobbyLevel"));

    if (bIsMainMenu)
    {
        ApplyInputProfile(EInputProfile::Menu);
        // show main menus
        return;
    }

    if (bIsLobby)
    {
        ApplyInputProfile(EInputProfile::Menu);
        EnsureReadyButton();              // create + AddToViewport if needed
        return;
    }

    // gameplay maps
    EnsureReadyButton();
    ApplyInputProfile(EInputProfile::Gameplay);
}


void AMyPlayerController::TravelToLobby_Implementation()
{
    if(GetWorld())
    {
        GetWorld()->ServerTravel(TEXT("/Game/GameAssets/Levels/LobbyLevel?listen"));

    }
}

void AMyPlayerController::EnsureReadyButton()
{
	const FString Map = UGameplayStatics::GetCurrentLevelName(this, true);
    const bool bIsLobby = (Map == TEXT("LobbyLevel"));
	if (bIsLobby)
	{
		if (ReadyButtonWidgetClass && !ReadyButtonWidgetInstance)
		{
			ReadyButtonWidgetInstance = CreateWidget<UReadyButtonWidget>(this, ReadyButtonWidgetClass);
		}
		if (ReadyButtonWidgetInstance && !ReadyButtonWidgetInstance->IsInViewport())
		{
			ReadyButtonWidgetInstance->AddToViewport(2000);
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
        DeathScreenInstance = CreateWidget<UGameOverMenuWidget>(this, DeathScreenClass);
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
	EnsureRoundSplashWidget(); // create the widget
	if (!RoundSplashWidgetInstance) return;

	if (!RoundSplashWidgetInstance->IsInViewport()) // if instance valid and not in viewport add to viewport
	{
		RoundSplashWidgetInstance->AddToViewport(1000);
	}

	RoundSplashWidgetInstance->SetVisibility(ESlateVisibility::Visible); 
	RoundSplashWidgetInstance->SetRound(RoundNumber); // this 
	
    if (RoundNumber != LastIntroSoundRoundPlayed)
    {
        LastIntroSoundRoundPlayed = RoundNumber;
        PlayRoundIntroSound(RoundNumber); // (this already does thud + delayed voice)
    }


    if (UWorld* World = GetWorld())
    {
        auto& TM = World->GetTimerManager();
        TM.ClearTimer(RoundIntroHideTimerHandle);
        TM.SetTimer(
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


void AMyPlayerController::TogglePauseMenu()
{
    if (!IsLocalController())
    {
        return;
    }

    bPauseMenuOpen ? HidePauseMenu() : ShowPauseMenu();
}



void AMyPlayerController::ShowPauseMenu()
{
    if (!PauseMenuWidgetClass) return;

	if(!PauseMenuWidgetInstance)
    {
        PauseMenuWidgetInstance = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);
    }
    if (!PauseMenuWidgetInstance)
    {
        return;
    }

    if(!PauseMenuWidgetInstance->IsInViewport())
	{
		PauseMenuWidgetInstance->AddToViewport(9999);
	}
    bPauseMenuOpen = true;

    // Only pause the world in standalone; in network, pause should be local UI only.
	// PlayerController never runs on a dedicated server
    const bool bIsStandalone = (GetNetMode() == NM_Standalone);
    const bool bIsMultiplayer = !bIsStandalone;

	PauseMenuWidgetInstance->ConfigureForMultiplayer(bIsMultiplayer);


    if (bIsStandalone)
    {
        UGameplayStatics::SetGamePaused(this, true);
    }

	bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget());
    SetInputMode(Mode);
}

void AMyPlayerController::HidePauseMenu()
{
    if (GetNetMode() == NM_Standalone)
    {
        UGameplayStatics::SetGamePaused(this, false);
    }

    if (PauseMenuWidgetInstance && PauseMenuWidgetInstance->IsInViewport())
    {
        PauseMenuWidgetInstance->RemoveFromParent();
    }

    bPauseMenuOpen = false;

	ApplyInputProfile(EInputProfile::Gameplay);
}

void AMyPlayerController::QuitToMainMenuFromPause()
{

	UGameplayStatics::OpenLevel(this, FName("MainMenu_Level"));
}

void AMyPlayerController::RequestRestartLevel()
{
	// adjust for multiplayer- only server should request restart
	const FName ZombiesLevel(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, ZombiesLevel);
}

void AMyPlayerController::GoToMainMenu()
{
	// adjust for multiplayer
    if (GetNetMode() == NM_Standalone)
    {
        UGameplayStatics::OpenLevel(this, FName("MainMenu_Level"));
        return;
    }

	if(HasAuthority())
	{
		// Server: kill the whole game
        UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit,true);
	}

	else
    {
        // Client: disconnect and return to menu
        ClientReturnToMainMenuWithTextReason(FText::FromString("Server ended the game"));
    }

}


