// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BaseGameState.h"
#include "DeathMatchGameState.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EMatchPhase : uint8 {
    Waiting     UMETA(DisplayName="Waiting"),
    InProgress  UMETA(DisplayName="In Progress"),
    PostMatch   UMETA(DisplayName="Post Match")
};

UCLASS()
class MYZOMBIES_API ADeathMatchGameState : public ABaseGameState
{
    GENERATED_BODY()

public:
    ADeathMatchGameState();

    UPROPERTY(ReplicatedUsing=OnRep_MatchPhase, BlueprintReadOnly, Category="Match")
    EMatchPhase MatchPhase = EMatchPhase::Waiting;

    UFUNCTION()
    void OnRep_MatchPhase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};