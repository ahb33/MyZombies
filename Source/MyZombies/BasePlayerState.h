// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"



/// make ABasePlayerState abstract so it cant be instantiated 
UCLASS(Abstract)
class MYZOMBIES_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:


	// Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_PlayerStats(); // sigle notify that updates HUD once 

	virtual int32 GetPlayerKills() const { return Kills; }
	virtual int32 GetPlayerDeaths() const { return Deaths; }
	virtual int32 GetPlayerScores() const { return Scores; }

private: 


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerStats, Category="Stats", meta=(AllowPrivateAccess="true"))
	int32 Kills = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerStats, Category="Stats", meta=(AllowPrivateAccess="true"))
	int32 Deaths = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerStats, Category="Stats", meta=(AllowPrivateAccess="true"))
	int32 Scores = 0;

protected:

    virtual void HandlePlayerStatsChanged() {} // virtual hook that runs whenever K/D/Score change
};
