// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerState.h"
#include "DeathMatchPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API ADeathMatchPlayerState : public ABasePlayerState
{
	GENERATED_BODY()
	
public:

	virtual void HandlePlayerStatsChanged() override;
	
	virtual void AddKill() { ++Kills; OnRep_PlayerStats(); }
	virtual void AddDeath() { ++Deaths; OnRep_PlayerStats(); }

private:

};
