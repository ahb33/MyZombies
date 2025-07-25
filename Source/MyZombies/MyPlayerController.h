// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"
/**
 * 
 */

class AMyHUD;
class ULobbyMenuWidget;
class UUserWidget;


UCLASS()
class MYZOMBIES_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    AMyPlayerController();

    virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

    void HandleReadyInput();

    void SetHUDHealth(float CurrentHealth, float MaxHealth);
    void SetHUDAmmo(int32 Ammo);
    void SetHUDMagAmmo(int32 AmmoInMag);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void TravelToLobby();

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetPlayerReady();


protected:
    UPROPERTY(EditAnywhere, Category = "HUD")
    AMyHUD* MyPlayerHUD;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> ReadyButtonWidgetClass;

    UPROPERTY()
    class UWBP_ReadyButtonWidget* ReadyButtonWidgetInstance;
};
