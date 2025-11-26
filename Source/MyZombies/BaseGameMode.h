// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"


class AMyPlayerController;
UCLASS(minimalapi)
class ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;
	AActor* ChoosePlayerStart(AController* Player);
	virtual void EndGame(bool bPlayerWon);
	// Function to be overridden in child classes for custom logic
	virtual void CheckEnemiesAlive();
	virtual void RequestSpawn(AController* Controller);

	bool bUseSeamlessTravel;

	// --- Damage policy toggles ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage Rules") bool bAllowPlayerVsPlayer = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage Rules") bool bAllowAIVsAI = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage Rules") bool bAllowFriendlyFireWithinTeam = false;

	// Central allow/deny hook (server)
    UFUNCTION()
    bool IsDamageAllowed(const AActor* Causer, const AActor* Target) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
    TArray<AActor*> AvailableSpawnPoints;

    UPROPERTY()
    TArray<AActor*> UsedSpawnPoints;

	UPROPERTY(EditAnywhere, Category = "EndGameWidgets")
	TSubclassOf<class UUserWidget> YouWonWidgetClass;

	UPROPERTY(EditAnywhere, Category = "EndGameWidgets")
	TSubclassOf<class UUserWidget> YouDiedWidgetClass;

	float RespawnDelay = 4.0f;
	
	
};



