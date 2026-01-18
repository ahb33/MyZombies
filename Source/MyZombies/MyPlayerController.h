// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
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
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;


public:
	// -------------------- Common Menu (used by any mode) --------------------
	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestRestartLevel();

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void GoToMainMenu();

	// add pause menu
	// pause should have 2 options : exi

	// Server calls this on the owning client when Zombies + killed by zombie.
	UFUNCTION(Client, Reliable)
	void Client_ShowDeathScreen();

	UFUNCTION(BlueprintCallable) void ShowDeathScreenLocal(); // local-only UI creator (used by Client RPC + OnRep fallback)

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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	class AMyHUD* GetMyHUD();

    void HandleRoundStateChanged(int32 RoundNumber, ERoundPhase Phase);
	void BindRoundDelegate(); // your delegate binding helper

	void InitRoundUI(); // Create (if needed) and keep the persistent corner “Round X” widget on-screen.
	void UpdateZombiesRoundWidget(int32 Round); // Push the latest round value into the persistent corner widget (calls Ensure first).
    void ShowRoundIntroSplashWidget(int32 RoundNumber);
	void PlayRoundIntroSound(int32 Round); // Play BOOM + “Round X” VO locally as part of the intro.

	UFUNCTION()
	void HandlePlayerDeath(); // needed for dynamic multicast
	
private:

	UPROPERTY(Transient)
	AMyHUD* MyPlayerHUD = nullptr;

	UPROPERTY(Transient)
	bool bDeathVisible = false;

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
	UZombiesRoundWidget* RoundHUDWidgetInstance  = nullptr; // ZombiesRoundWidgetInstance

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

	FTimerHandle RoundVoiceTimerHandle;
	FTimerHandle RoundIntroHideTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	float RoundIntroWidgetDuration = 2.0f;

	UPROPERTY()
    TObjectPtr<AZombiesGameState> CachedZGS = nullptr;

};
