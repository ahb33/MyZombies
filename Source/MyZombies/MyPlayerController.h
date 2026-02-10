// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "BaseGameState.h"
#include "ZombiesTypes.h"
#include "MyPlayerController.generated.h"


/**
 * Custom PlayerController:
 * - Handles HUD updates
 * - Manages input (ready state, lobby travel)
 * - Server RPCs for multiplayer flow
 */
class AMyHUD;
class ULobbyMenuWidget;
class UUserWidget;
class UReadyButtonWidget;
class UYouDiedMenuWidget;
class UGameOverMenuWidget;
class UZombiesRoundWidget;
class UAudioComponent;
class AZombiesGameState;
class UMenuUIManager;
class UPauseMenuWidget;


UCLASS()
class MYZOMBIES_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	UMenuUIManager* GetMenuUI() const { return MenuUI; }

	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginPlayingState();

	// -------------------- Common Menu (used by any mode) --------------------
	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestRestartLevel();

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void GoToMainMenu();


	// pause should have 2 options
	void HidePauseMenu();


	UFUNCTION(BlueprintCallable) 
	void ShowDeathScreenLocal(); // local-only UI creator (used by Client RPC + OnRep fallback)

	// HUD Helpers
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMagAmmo(int32 AmmoInMag);
	void UpdateHUDKillDeath(int32 Kills, int32 Deaths);

	// Lobby Ready
	void HandleReadyInput();

	UFUNCTION(Server, Reliable)
	void TravelToLobby();
	void UpdateUIForCurrentMap();
	void EnsureReadyButton();

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerReady();


	
private:
	// Binding / state sync
	void TryBindToGameState();
	void UnbindFromGameState();
	void SyncFromCachedState();

	void HandleInputProfileChanged(EInputProfile Profile);
	void HandleMatchPhaseChanged(EMatchPhase Phase);
	void HandleMatchModeChanged(EMatchMode Mode);

	void HandleRoundNumberChanged(int32 RoundNumber);

	// Input application
	void ApplyInputProfile(EInputProfile Profile);
	// UFUNCTION()
	// void ApplyMenuInputAndFocus(UWidget* FocusTarget, bool bUIOnly);

	// Zombies UI
	void EnsureRoundHUDWidget();
	void EnsureRoundSplashWidget();
	void HideRoundIntroSplashWidget();
	void ShowRoundIntroSplashWidget(int32 RoundNumber);

	void TogglePauseMenu();
	void ShowPauseMenu();
	void QuitToMainMenuFromPause();

	// Audio
	void PlayRoundIntroSound(int32 RoundNumber);
	void PlayRoundVoiceSound(USoundBase* VoiceSound);

	AMyHUD* GetMyHUD();
	
private:

	// Assign these in BP
	UPROPERTY(EditDefaultsOnly, Category = "UI|Menus")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Menus")
	TSubclassOf<UUserWidget> SoloMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Menus")
	TSubclassOf<UUserWidget> GameModeSelectionMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Menus")
	TSubclassOf<UUserWidget> CreateSessionMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Menus")
	TSubclassOf<UUserWidget> JoinSessionMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="UI|Menus")
	TSubclassOf<UUserWidget> MultiplayerMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="UI|Menus")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMenuUIManager> MenuUI;

	// Cached references
	UPROPERTY(Transient)
	TObjectPtr<AMyHUD> MyPlayerHUD = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ABaseGameState> CachedBGS = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AZombiesGameState> CachedZGS = nullptr;

	// Delegate handles (avoid RemoveAll noise)
	FDelegateHandle InputProfileChangedHandle;
	FDelegateHandle MatchPhaseChangedHandle;
	FDelegateHandle MatchModeChangedHandle;
	FDelegateHandle RoundNumberChangedHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI|Lobby", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UUserWidget> ReadyButtonWidgetClass;
	UPROPERTY(Transient)
	TObjectPtr<UReadyButtonWidget> ReadyButtonWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="UI|Death")
	TSubclassOf<UUserWidget> DeathScreenClass;
	UPROPERTY(Transient)
	TObjectPtr<UGameOverMenuWidget> DeathScreenInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> PauseMenuWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="UI|Round Info")
	TSubclassOf<UUserWidget> RoundHUDWidgetClass;
	UPROPERTY(Transient)
	TObjectPtr<UZombiesRoundWidget> RoundHUDWidgetInstance  = nullptr; 

	UPROPERTY(EditDefaultsOnly, Category="UI|Round Info")
	TSubclassOf<UUserWidget> RoundSplashWidgetClass;
	UPROPERTY(Transient)
	TObjectPtr<UZombiesRoundWidget> RoundSplashWidgetInstance = nullptr;


	int32 LastIntroSoundRoundPlayed = INDEX_NONE;
    UPROPERTY(EditDefaultsOnly, Category="Audio")
    TObjectPtr<USoundBase> RoundThudSound = nullptr;

    // Index = Round-1
    UPROPERTY(EditDefaultsOnly, Category="Audio")
    TArray<TObjectPtr<USoundBase>> RoundVoiceSounds;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> RoundIntroThudComp = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> RoundIntroVoiceComp = nullptr;

	UPROPERTY(EditDefaultsOnly) 
	float RoundIntroWidgetDuration = 2.0f;

	FTimerHandle RoundVoiceTimerHandle;
	FTimerHandle RoundIntroHideTimerHandle;

	bool bReadyRequestInFlight = false;

	FTimerHandle BindRetryTimerHandle;
	int32 BindRetryCount = 0;

	int32 CachedRoundNumber = 1;

	bool bPauseMenuOpen = false;
	
};