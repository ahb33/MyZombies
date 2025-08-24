// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameState.h"
#include "Net/UnrealNetWork.h"

ADeathMatchGameState::ADeathMatchGameState() {}

void ADeathMatchGameState::OnRep_MatchPhase()
{
    // TODO: HUD/announcer update when phase changes
}

void ADeathMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADeathMatchGameState, MatchPhase);
}
