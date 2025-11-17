// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BaseGameState.generated.h"

/**
 * 
 */

UENUM(BlueprintType) enum class EMatchMode : uint8 { Zombies, Deathmatch };

UCLASS()
class MYZOMBIES_API ABaseGameState : public AGameState
{
	GENERATED_BODY()

public:

  UFUNCTION(BlueprintPure) 
  EMatchMode GetMatchMode() const { return MatchMode; }

  UFUNCTION(BlueprintAuthorityOnly)
  void SetMatchMode(EMatchMode InMode) { if (HasAuthority()) { MatchMode = InMode; } }


protected:

  UPROPERTY(ReplicatedUsing=OnRep_MatchMode, BlueprintReadOnly, Category="Match")
  EMatchMode MatchMode = EMatchMode::Zombies; // set default

  UFUNCTION() void OnRep_MatchMode() {}
  virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
