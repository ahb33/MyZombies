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

    void PossessCharacterIfNone(class APlayerController* PC);
    void SetupInputForGameplay(class APlayerController* PC);



protected:
	virtual void BeginPlay() override;

	virtual void CheckEnemiesAlive() override;

    void StartNextWave();
    void ApplyLevelModifiers();

	void BindZombieDeathEvents();


	UPROPERTY(EditAnywhere, Category="AI Stats")
	UDataTable* AIDifficultyTable;

private:
	int32 CurrentLevel;
	int32 RemainingEnemies;

	int32 NumberOfZombiesForCurrentLevel;
	
};
