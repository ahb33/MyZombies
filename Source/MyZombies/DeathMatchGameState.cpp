// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameState.h"
#include "Net/UnrealNetwork.h"

ADeathMatchGameState::ADeathMatchGameState() {}


void ADeathMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADeathMatchGameState, MatchPhase);
}


void ADeathMatchGameState::OnRep_MatchPhase()
{

}



