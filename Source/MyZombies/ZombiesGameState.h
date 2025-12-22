// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameState.h"
#include "ZombiesTypes.h"
#include "ZombiesGameState.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API AZombiesGameState : public ABaseGameState
{
	GENERATED_BODY()


public: 

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	FORCEINLINE int32 GetRoundNumber() const {return RoundNumber;}
	FORCEINLINE ERoundPhase GetRoundPhase() const {return RoundPhase;}

	FORCEINLINE void SetRoundNumber(int32 InRound) { if (HasAuthority()) RoundNumber = InRound; }
	FORCEINLINE void SetRoundPhase(ERoundPhase InPhase) { if (HasAuthority()) RoundPhase = InPhase; }


private:

	UPROPERTY(Replicated) int32 RoundNumber;
	UPROPERTY(Replicated) ERoundPhase RoundPhase;

	
};
