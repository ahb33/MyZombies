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
class UWBP_ReadyButtonWidget;

UCLASS()
class MYZOMBIES_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	// --- Core overrides ---
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// --- HUD Update Functions ---
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMagAmmo(int32 AmmoInMag);
	void UpdateHUDKillDeath(int32 Kills, int32 Deaths);

	// Helper to cache HUD
	AMyHUD* GetMyHUD();

	// --- Input / Lobby ---
	void HandleReadyInput();

	// --- Server RPCs ---
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TravelToLobby();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetPlayerReady();

protected:
	// --- HUD Reference (runtime only, not editable in editor) ---
	UPROPERTY()
	AMyHUD* MyPlayerHUD = nullptr;

	// --- Lobby UI ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> ReadyButtonWidgetClass;

	UPROPERTY()
	UWBP_ReadyButtonWidget* ReadyButtonWidgetInstance = nullptr;
};