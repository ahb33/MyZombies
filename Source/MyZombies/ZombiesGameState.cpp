// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombiesGameState.h"
#include "Net/UnrealNetwork.h"

void AZombiesGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AZombiesGameState, RoundState);

}

void AZombiesGameState::ServerSetRoundState(int32 NewRoundNumber, ERoundPhase NewPhase)
{
    if(!HasAuthority()) return;

    RoundState.RoundNumber = NewRoundNumber;
    RoundState.Phase = NewPhase;

    BroadcastRoundState();

    ForceNetUpdate();
}

void AZombiesGameState::BroadcastRoundState()
{

    OnRoundStateChanged.Broadcast(RoundState.RoundNumber, RoundState.Phase);

}

void AZombiesGameState::OnRep_RoundState()
{
    BroadcastRoundState();
}
