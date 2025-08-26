// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "DeathMatchPlayerState.h"



void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{


  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ABasePlayerState, Kills);
  DOREPLIFETIME(ABasePlayerState, Deaths);
  DOREPLIFETIME(ABasePlayerState, Scores);

}

void ABasePlayerState::OnRep_PlayerStats() 
{
    HandlePlayerStatsChanged();
}

