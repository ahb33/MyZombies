// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "BaseGameState.h"
#include "LobbyGameMode.h"   
#include "MyPlayerController.generated.h"

class AMyHUD;
class UPlayerUISubsystem;
class UUserWidget;
class UReadyButtonWidget;
 
class AZombiesGameState;

UCLASS()
class MYZOMBIES_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void BeginPlayingState() override;

	void InitLocalUI();

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestRestartLevel();

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void GoToMainMenu();

	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMagAmmo(int32 AmmoInMag);

	void HandleReadyInput();

	UFUNCTION(Server, Reliable)
	void TravelToLobby();

	void UpdateUIForCurrentMap();
	void EnsureReadyButton();

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerReady();

	UFUNCTION(BlueprintPure, Category = "UI")
	UPlayerUISubsystem* GetUISubsystem() const { return UISubsystem.Get(); }

	void TogglePauseMenu();

	/** Base profile application (no explicit focus). */
	void ApplyInputProfile(EInputProfile Profile);

	/** Profile application that also sets UI focus (recommended for menus). */
	void ApplyInputProfile(EInputProfile Profile, UUserWidget* FocusWidget);

private:
	void TryBindToGameState();
	void UnbindFromGameState();
	void SyncFromCachedState();

	void HandleInputProfileChanged(EInputProfile Profile);
	void HandleMatchPhaseChanged(EMatchPhase Phase);
	void HandleMatchModeChanged(EMatchMode Mode);
	void HandleRoundNumberChanged(int32 RoundNumber);

	void QuitToMainMenuFromPause();
	AMyHUD* GetMyHUD();

private:
	UPROPERTY()
	TObjectPtr<UPlayerUISubsystem> UISubsystem;

	UPROPERTY(Transient)
	TObjectPtr<AMyHUD> MyPlayerHUD = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ABaseGameState> CachedBGS = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AZombiesGameState> CachedZGS = nullptr;

	FDelegateHandle InputProfileChangedHandle;
	FDelegateHandle MatchPhaseChangedHandle;
	FDelegateHandle MatchModeChangedHandle;
	FDelegateHandle RoundNumberChangedHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI|Lobby", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UUserWidget> ReadyButtonWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UReadyButtonWidget> ReadyButtonWidgetInstance = nullptr;

	bool bReadyRequestInFlight = false;
	FTimerHandle BindRetryTimerHandle;
	int32 BindRetryCount = 0;

	int32 CachedRoundNumber = 1;
	bool bPauseMenuOpen = false;
	bool bUIReady = false;
	EMatchPhase CachedPhase = EMatchPhase::None;
};