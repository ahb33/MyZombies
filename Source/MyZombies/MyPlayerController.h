// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
	class AMyHUD* GetMyHUD();

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
};