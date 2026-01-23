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
	void BeginWaveActive();
	void OnZombieSpawned();

protected:
	virtual void BeginPlay() override;

    void StartNextWave();
    void ApplyLevelModifiers();

	void TryAdvanceWave();

	UPROPERTY(EditAnywhere, Category="AI Stats")
	UDataTable* AIDifficultyTable;

private:
	int32 CurrentLevel;
	int32 RemainingEnemies;

	int32 NumberOfZombiesForCurrentLevel;

	FTimerHandle WaveIntroTimer;
	float RoundIntroDelay = 2.0f; // tweak
	int32 ZombiesToSpawnThisWave = 0;
	int32 ZombiesSpawnedThisWave = 0;
	
};
