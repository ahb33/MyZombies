// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
#include "DeathMatchGameMode.generated.h"

/**
 * 
 */

class USoundBase;
class AController;
class ADeathMatchGameState;

UCLASS()
class MYZOMBIES_API ADeathMatchGameMode : public ABaseGameMode
{
	GENERATED_BODY()
	
public:

	ADeathMatchGameMode();

	virtual void BeginPlay() override;

	virtual void HandlePlayerDeath(AController* Victim, AController* Killer) override;
	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rounds")
	int32 MaxRounds = 5;

private:
	int32 CurrentRound = 1;

	void EndRound();
	void StartNextRoundOrEndMatch();
};