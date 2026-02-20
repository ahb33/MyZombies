// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"

#include "MyHUD.h"
#include "LobbyPlayerState.h"
#include "ReadyButtonWidget.h"
#include "Components/ProgressBar.h"
#include "CharacterStats.h"
#include "BaseGameState.h"
#include "UIHelpers.h"
#include "KillDeathStats.h"
#include "PlayerUISubsystem.h"
#include "ZombiesGameState.h"
#include "MenuUIManager.h"
#include "MenuIds.h"
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
	GetWorldTimerManager().SetTimerForNextTick(this, &AMyPlayerController::InitLocalUI);
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent) return;

	InputComponent->BindAction("StartGame", IE_Pressed, this, &AMyPlayerController::HandleReadyInput);

	auto& Binding = InputComponent->BindAction("Pause", IE_Pressed, this, &AMyPlayerController::TogglePauseMenu);
	Binding.bExecuteWhenPaused = true;
}

void AMyPlayerController::InitLocalUI()
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AMyPlayerController::InitLocalUI);
		return;
	}

	UISubsystem = LP->GetSubsystem<UPlayerUISubsystem>();
	if (!UISubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitLocalUI - UISubsystem not found. Retrying next tick."));
		GetWorldTimerManager().SetTimerForNextTick(this, &AMyPlayerController::InitLocalUI);
		return;
	}

	UISubsystem->SetOwnerPC(this);
	bUIReady = true;

	HandleMatchPhaseChanged(CachedPhase);
	HandleRoundNumberChanged(CachedRoundNumber);

	UpdateUIForCurrentMap();
}

void AMyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	TryBindToGameState();

	const FString Map = UGameplayStatics::GetCurrentLevelName(this, true);
	if (Map == TEXT("MainMenu_Level")) return;

	UpdateUIForCurrentMap();
}

void AMyPlayerController::TryBindToGameState()
{
	UnbindFromGameState();

	UWorld* World = GetWorld();
	if (!World) return;

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

	InputProfileChangedHandle = CachedBGS->OnInputProfileChanged.AddUObject(this, &AMyPlayerController::HandleInputProfileChanged);
	MatchPhaseChangedHandle = CachedBGS->OnMatchPhaseChanged.AddUObject(this, &AMyPlayerController::HandleMatchPhaseChanged);
	MatchModeChangedHandle = CachedBGS->OnMatchModeChanged.AddUObject(this, &AMyPlayerController::HandleMatchModeChanged);

	CachedZGS = World->GetGameState<AZombiesGameState>();
	if (CachedZGS)
	{
		RoundNumberChangedHandle = CachedZGS->OnRoundNumberChanged.AddUObject(this, &AMyPlayerController::HandleRoundNumberChanged);
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
	if (Map == TEXT("MainMenu_Level")) return;

	ApplyInputProfile(Profile);
	EnsureReadyButton();
}

void AMyPlayerController::HandleMatchModeChanged(EMatchMode Mode)
{
	if (!UISubsystem) return;

	if (Mode != EMatchMode::Zombies)
	{
		UISubsystem->HideRoundHUD();
		UISubsystem->HideRoundIntroSplash();
	}
}

void AMyPlayerController::HandleMatchPhaseChanged(EMatchPhase Phase)
{
	CachedPhase = Phase;

	if (!bUIReady || !UISubsystem) return;

	switch (Phase)
	{
		case EMatchPhase::GameOver:
			UISubsystem->ShowDeathScreenLocal();
			break;

		case EMatchPhase::Intro:
			if (CachedBGS && CachedBGS->GetMatchMode() == EMatchMode::Zombies)
			{
				UISubsystem->ShowRoundIntroSplashWidget(CachedRoundNumber);
			}
			break;

		default:
			UISubsystem->HideRoundIntroSplash();
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

	if (!UISubsystem) return;

	UISubsystem->UpdateRoundHUD(CachedRoundNumber);

	if (CachedBGS->GetMatchPhase() == EMatchPhase::Intro)
	{
		UISubsystem->ShowRoundIntroSplashWidget(CachedRoundNumber);
	}
}

void AMyPlayerController::ApplyInputProfile(EInputProfile Profile)
{
	ApplyInputProfile(Profile, nullptr);
}

void AMyPlayerController::ApplyInputProfile(EInputProfile Profile, UUserWidget* FocusWidget)
{
	switch (Profile)
	{
		case EInputProfile::Menu:
		{
            const FString Map = UGameplayStatics::GetCurrentLevelName(this, true);

            // Lobby should allow movement + UI interaction + action mappings (Enter).
            if (Map == TEXT("LobbyLevel"))
            {
                UIHelpers::ApplyGameAndUI(this, FocusWidget);
            }
            else
            {
                UIHelpers::ApplyUIOnly(this, FocusWidget);
            }
            break;

		}

        case EInputProfile::Gameplay:
        default:
            UIHelpers::ApplyGameOnly(this);
            break;
	}
}

void AMyPlayerController::HandleReadyInput()
{
	if (bReadyRequestInFlight) return;
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
	const bool bIsLobby = (Map == TEXT("LobbyLevel"));

	ApplyInputProfile((bIsMainMenu || bIsLobby) ? EInputProfile::Menu : EInputProfile::Gameplay);

	if (bIsMainMenu)
	{
		if (!UISubsystem) return;

		UMenuUIManager* Manager = UISubsystem->GetMenuManager();
		if (!Manager) return;

		if (Manager->GetActiveMenuID() != MenuIds::MainMenu)
		{
			Manager->ShowMenu(MenuIds::MainMenu);
		}
		return;
	}

	EnsureReadyButton();

	if (bIsLobby && ReadyButtonWidgetInstance && ReadyButtonWidgetInstance->IsInViewport())
	{
		ReadyButtonWidgetInstance->SetUserFocus(this);
		ReadyButtonWidgetInstance->SetKeyboardFocus();
	}
}

void AMyPlayerController::TravelToLobby_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("/Game/GameAssets/Levels/LobbyLevel?listen"));
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
			ReadyButtonWidgetInstance->SetUserFocus(this);
			ReadyButtonWidgetInstance->SetKeyboardFocus();
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

        // Critical: readiness changes must re-evaluate lobby start conditions.
        if (UWorld* World = GetWorld())
        {
            if (ALobbyGameMode* GM = World->GetAuthGameMode<ALobbyGameMode>())
            {
                GM->CheckLobbyReady(); // ensure this is public in LobbyGameMode.h
            }
        }
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

void AMyPlayerController::TogglePauseMenu()
{
	if (!IsLocalController() || !UISubsystem) return;

	if (bPauseMenuOpen)
	{
		UISubsystem->HidePauseMenu();
	}
	else
	{
		UISubsystem->ShowPauseMenu();
	}
	bPauseMenuOpen = !bPauseMenuOpen;
}

void AMyPlayerController::QuitToMainMenuFromPause()
{
	GoToMainMenu();
}

void AMyPlayerController::RequestRestartLevel()
{
	const FName CurrentLevel(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, CurrentLevel);
}

void AMyPlayerController::GoToMainMenu()
{
	if (GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(this, FName("MainMenu_Level"));
		return;
	}

	if (HasAuthority())
	{
		if (AGameModeBase* GM = UGameplayStatics::GetGameMode(this))
		{
			GM->ReturnToMainMenuHost();
		}
		else if (UWorld* World = GetWorld())
		{
			World->ServerTravel(TEXT("/Game/Maps/MainMenu_Level?listen"));
		}
		return;
	}

	ClientReturnToMainMenuWithTextReason(FText::FromString("Server ended the game"));
}



