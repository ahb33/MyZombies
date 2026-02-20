// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameState.h"
#include "ZombiesGameState.generated.h"

/**
 * 
 */


DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoundNumberChanged, int32);


UCLASS()
class MYZOMBIES_API AZombiesGameState : public ABaseGameState
{
	GENERATED_BODY()


public: 

	FOnRoundNumberChanged OnRoundNumberChanged;

	UFUNCTION(BlueprintPure, Category="Zombies|Round")
	int32 GetRoundNumber() const { return RoundNumber; }

	/** Server-only: GameMode calls when starting next wave/round. */
	UFUNCTION(BlueprintCallable, Category="Zombies|Round")
	void SetRoundNumber(int32 NewRoundNumber);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_RoundNumber, BlueprintReadOnly, Category="Zombies|Round")
	int32 RoundNumber = 1;

	UFUNCTION()
	void OnRep_RoundNumber();

private:
	void BroadcastRoundNumber();
};
