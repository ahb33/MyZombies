// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "ZombiesTypes.h"
#include "BaseGameState.h"
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
class UZombiesRoundWidget;
class UAudioComponent;
class AZombiesGameState;


UCLASS()
class MYZOMBIES_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// -------------------- Common Menu (used by any mode) --------------------
	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestRestartLevel();

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void GoToMainMenu();

	// add pause menu
	// pause should have 2 options : exi

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

	// Zombies UI
	void EnsureRoundHUDWidget();
	void EnsureRoundSplashWidget();
	void HideRoundIntroSplashWidget();
	void ShowRoundIntroSplashWidget(int32 RoundNumber);

	// Audio
	void PlayRoundIntroSound(int32 RoundNumber);
	void PlayRoundVoiceSound(USoundBase* VoiceSound);

	AMyHUD* GetMyHUD();
	
private:
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
	UReadyButtonWidget* ReadyButtonWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="UI|Death")
	TSubclassOf<UUserWidget> DeathScreenClass;
	UPROPERTY(Transient)
	UYouDiedMenuWidget* DeathScreenInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="UI|Round Info")
	TSubclassOf<UUserWidget> RoundHUDWidgetClass;
	UPROPERTY(Transient)
	UZombiesRoundWidget* RoundHUDWidgetInstance  = nullptr; 

	UPROPERTY(EditDefaultsOnly, Category="UI|Round Info")
	TSubclassOf<UUserWidget> RoundSplashWidgetClass;
	UPROPERTY(Transient)
	UZombiesRoundWidget* RoundSplashWidgetInstance = nullptr;

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

	FTimerHandle BindRetryTimerHandle;
	int32 BindRetryCount = 0;

	int32 CachedRoundNumber = 1;
};
