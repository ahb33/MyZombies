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

	void OnMatchStart();
	void OnMatchEnd();
	void OnPlayerKilled(AController* Attacker, AController* Victim);
	void RequestSpawn(AController* Victim);
	

private: 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Start Cue ", meta=(AllowPrivateAccess="true")) 
	USoundBase* MatchStartCue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Start Cue ", meta=(AllowPrivateAccess="true")) 
	USoundBase* MatchOverCue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Score Target ", meta=(AllowPrivateAccess="true"))
	int32 ScoreToWin; 



};
