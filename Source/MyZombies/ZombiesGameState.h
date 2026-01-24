// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameState.h"
#include "ZombiesTypes.h"
#include "ZombiesGameState.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FRoundState
{
	GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 RoundNumber = 1;

    UPROPERTY(BlueprintReadOnly)
    ERoundPhase Phase = ERoundPhase::Intro;

};


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRoundStateChanged, int32, ERoundPhase);


UCLASS()
class MYZOMBIES_API AZombiesGameState : public ABaseGameState
{
	GENERATED_BODY()


public: 

	FOnRoundStateChanged OnRoundStateChanged;

	FORCEINLINE int32 GetRoundNumber() const {return RoundState.RoundNumber;}
	FORCEINLINE ERoundPhase GetRoundPhase() const {return RoundState.Phase;}

    void ServerSetRoundState(int32 NewRoundNumber, ERoundPhase NewPhase);

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(ReplicatedUsing=OnRep_RoundState)
    FRoundState RoundState;

	UFUNCTION()
    void OnRep_RoundState();

private:
    void BroadcastRoundState();

};
