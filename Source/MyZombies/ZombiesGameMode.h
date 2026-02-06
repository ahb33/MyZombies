// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
#include "ZombiesGameMode.generated.h"

/**
 * 
 */


UCLASS()
class MYZOMBIES_API AZombiesGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AZombiesGameMode();

	UFUNCTION()
	void OnZombieKilled();
	void OnZombieSpawned();
	virtual void HandlePlayerDeath(AController* Victim, AController* Killer) override;

private:

	void BeginWaveActive();
	virtual void BeginPlay() override;
    void StartNextWave();
    void ApplyLevelModifiers();

	void TryAdvanceWave();

	UPROPERTY(EditAnywhere, Category="AI Stats")
	TObjectPtr<UDataTable> AIDifficultyTable = nullptr;


	int32 CurrentLevel = 1;
	int32 RemainingEnemies = 0;

	int32 NumberOfZombiesForCurrentLevel;

	FTimerHandle WaveIntroTimer;

	UPROPERTY(EditAnywhere, Category="Waves")
	float WaveIntroDelay = 2.0f;

	int32 ZombiesToSpawnThisWave = 0;
	int32 ZombiesSpawnedThisWave = 0;


	
};
