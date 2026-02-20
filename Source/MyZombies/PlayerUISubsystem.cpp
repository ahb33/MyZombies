#include "PlayerUISubsystem.h"

#include "MyPlayerController.h"
#include "MenuUIManager.h"
#include "MyGameInstance.h"
#include "MenuIds.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "PauseMenuWidget.h"
#include "GameOverMenuWidget.h"
#include "ZombiesRoundWidget.h"
#include "Components/AudioComponent.h"
#include "UIHelpers.h"

void UPlayerUISubsystem::SetOwnerPC(AMyPlayerController* InPC)
{
	OwnerPC = InPC;
}
void UPlayerUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PullWidgetClassesFromGameInstance();
}

void UPlayerUISubsystem::PullWidgetClassesFromGameInstance()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const UMyGameInstance* MyGI = Cast<UMyGameInstance>(GI);
	if (!MyGI) return;

	MainMenuWidgetClass = MyGI->GetMainMenuWidgetClass();
	SoloMenuWidgetClass = MyGI->GetSoloMenuWidgetClass();
	GameModeSelectionMenuWidgetClass = MyGI->GetGameModeSelectionMenuWidgetClass();
	CreateSessionMenuWidgetClass = MyGI->GetCreateSessionMenuWidgetClass();
	JoinSessionMenuWidgetClass = MyGI->GetJoinSessionMenuWidgetClass();
	MultiplayerMenuWidgetClass = MyGI->GetMultiplayerMenuWidgetClass();

	PauseMenuWidgetClass = MyGI->GetPauseMenuWidgetClass();
	DeathScreenWidgetClass = MyGI->GetDeathScreenWidgetClass();

	RoundSplashWidgetClass = MyGI->GetRoundSplashWidgetClass();
	RoundHUDWidgetClass = MyGI->GetRoundHUDWidgetClass();
	RoundThudSound = MyGI->GetRoundThudSound();
	RoundIntroWidgetDuration = MyGI->GetRoundIntroWidgetDuration();
}

void UPlayerUISubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundIntroHideTimerHandle);
	}

	if (IsValid(RoundIntroThudComp))
	{
		RoundIntroThudComp->Stop();
	}
	RoundIntroThudComp = nullptr;

	// Avoid touching invalid PC/widgets during teardown.
	if (OwnerPC.IsValid())
	{
		HidePauseMenu();
		HideRoundIntroSplash();
		HideRoundHUD();
		HideOverlay(DeathScreenInstance.Get(), true);
	}

	MenuUI = nullptr;
	OwnerPC.Reset();

	Super::Deinitialize();
}

UMenuUIManager* UPlayerUISubsystem::GetMenuManager()
{
	if (!MenuUI && OwnerPC.IsValid())
	{
		MenuUI = NewObject<UMenuUIManager>(this);
		if (MenuUI)
		{
			MenuUI->Init(OwnerPC.Get());

			MenuUI->RegisterMenu(MenuIds::MainMenu, MainMenuWidgetClass, 0);
			MenuUI->RegisterMenu(MenuIds::SoloMenu, SoloMenuWidgetClass, 0);
			MenuUI->RegisterMenu(MenuIds::GameModeSelection, GameModeSelectionMenuWidgetClass, 0);
			MenuUI->RegisterMenu(MenuIds::MultiplayerMenu, MultiplayerMenuWidgetClass, 0);
			MenuUI->RegisterMenu(MenuIds::CreateSession, CreateSessionMenuWidgetClass, 0);
			MenuUI->RegisterMenu(MenuIds::JoinSession, JoinSessionMenuWidgetClass, 0);
		}
	}

	return MenuUI.Get();
}

EInputProfile UPlayerUISubsystem::GetDesiredProfileForCurrentMap() const
{
	if (!OwnerPC.IsValid()) return EInputProfile::Gameplay;

	const FString Map = UGameplayStatics::GetCurrentLevelName(OwnerPC.Get(), true);
	const bool bMenuMap = (Map == TEXT("MainMenu_Level")) || (Map == TEXT("LobbyLevel"));
	return bMenuMap ? EInputProfile::Menu : EInputProfile::Gameplay;
}

void UPlayerUISubsystem::ShowOverlayInternal(
	TSubclassOf<UUserWidget> WidgetClass,
	UUserWidget*& InOutInstance,
	int32 ZOrder,
	bool bPauseWorld,
	bool bMultiplayerConfig,
	EOverlayInputMode InputMode)
{
	if (!OwnerPC.IsValid() || !WidgetClass) return;

	AMyPlayerController* PC = OwnerPC.Get();
	if (!IsValid(PC)) return;

	if (!IsValid(InOutInstance))
	{
		InOutInstance = CreateWidget<UUserWidget>(OwnerPC.Get(), WidgetClass);
	}
	if (!IsValid(InOutInstance)) return;

	if (!InOutInstance->IsInViewport())
	{
		InOutInstance->AddToViewport(ZOrder);
	}

	if (bMultiplayerConfig)
	{
		if (UPauseMenuWidget* PauseWidget = Cast<UPauseMenuWidget>(InOutInstance))
		{
			PauseWidget->ConfigureForMultiplayer(true);
		}
	}

	switch (InputMode)
	{
        case EOverlayInputMode::UIOnly:
            PC->ApplyInputProfile(EInputProfile::Menu, InOutInstance);
            break;

        case EOverlayInputMode::GameAndUI:
            UIHelpers::ApplyGameAndUI(PC, InOutInstance);
            break;

        case EOverlayInputMode::None:
        default:
            break;
	}

	if (bPauseWorld && OwnerPC->GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(PC, true);
	}
}

void UPlayerUISubsystem::HideOverlay(UUserWidget* WidgetInstance, bool bRestoreInput)
{
    if (!OwnerPC.IsValid() || !IsValid(WidgetInstance)) return;

    AMyPlayerController* PC = OwnerPC.Get();
    if (!IsValid(PC)) return;

    const bool bWasVisible = WidgetInstance->IsInViewport();
    if (bWasVisible)
    {
        WidgetInstance->RemoveFromParent();
    }

    if (bRestoreInput && bWasVisible)
    {
        PC->ApplyInputProfile(GetDesiredProfileForCurrentMap());
    }
}

void UPlayerUISubsystem::ShowPauseMenu()
{
    if (!OwnerPC.IsValid()) return;

    AMyPlayerController* PC = OwnerPC.Get();
    if (!IsValid(PC)) return;

    const bool bIsStandalone = (PC->GetNetMode() == NM_Standalone);
    const bool bIsMultiplayer = !bIsStandalone;

    ShowOverlay<UPauseMenuWidget>(
        PauseMenuWidgetClass,
        PauseMenuInstance,
        9999,
        bIsStandalone,
        bIsMultiplayer,
        bIsStandalone ? EOverlayInputMode::UIOnly : EOverlayInputMode::GameAndUI
    );
}

void UPlayerUISubsystem::HidePauseMenu()
{
	if (!OwnerPC.IsValid()) return;

	AMyPlayerController* PC = OwnerPC.Get();
    if (!IsValid(PC)) return;


	HideOverlay(PauseMenuInstance.Get(), true);

	if (OwnerPC->GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(OwnerPC.Get(), false);
	}
}

void UPlayerUISubsystem::ShowDeathScreenLocal()
{
	if (!DeathScreenWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowDeathScreenLocal: DeathScreenWidgetClass is NULL (set in BP_MyGameInstance)."));
		return;
	}

	ShowOverlay<UGameOverMenuWidget>(
		DeathScreenWidgetClass,
		DeathScreenInstance,
		1000,
		false,
		false,
		EOverlayInputMode::UIOnly
	);
}

void UPlayerUISubsystem::ShowRoundIntroSplashWidget(int32 RoundNumber)
{

	ShowOverlay<UZombiesRoundWidget>(
		RoundSplashWidgetClass,
		RoundSplashWidgetInstance,
		500,
		false,
		false,
		EOverlayInputMode::None
	);

	if (RoundSplashWidgetInstance)
	{
		RoundSplashWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
		RoundSplashWidgetInstance->SetRound(RoundNumber);
		PlayRoundIntroSound();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RoundIntroHideTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				HideRoundIntroSplash();
			}),
			RoundIntroWidgetDuration,
			false
		);
	}
}

void UPlayerUISubsystem::HideRoundIntroSplash()
{
	HideOverlay(RoundSplashWidgetInstance.Get(), false);
}

void UPlayerUISubsystem::PlayRoundIntroSound()
{
	if (!OwnerPC.IsValid()) return;

	AMyPlayerController* PC = OwnerPC.Get();
    if (!IsValid(PC)) return;


	if (IsValid(RoundIntroThudComp) && RoundIntroThudComp->IsPlaying())
	{
		RoundIntroThudComp->Stop();
	}

	if (!RoundThudSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayRoundIntroSound: RoundThudSound is NULL (set it in BP_MyGameInstance)."));
		return;
	}

	RoundIntroThudComp = UGameplayStatics::SpawnSound2D(OwnerPC.Get(), RoundThudSound);
}

void UPlayerUISubsystem::UpdateRoundHUD(int32 RoundNumber)
{

	if (GetDesiredProfileForCurrentMap() == EInputProfile::Menu)
    {
        HideRoundIntroSplash();
        return;
    }
	
	ShowOverlay<UZombiesRoundWidget>(
		RoundHUDWidgetClass,
		RoundHUDWidgetInstance,
		1000,
		false,
		false,
		EOverlayInputMode::None
	);

	if (RoundHUDWidgetInstance)
	{
		RoundHUDWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		RoundHUDWidgetInstance->SetRound(RoundNumber);
	}
}

void UPlayerUISubsystem::HideRoundHUD()
{
	HideOverlay(RoundHUDWidgetInstance.Get(), false);
}



