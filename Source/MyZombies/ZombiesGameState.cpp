// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombiesGameState.h"
#include "Net/UnrealNetwork.h"

void AZombiesGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AZombiesGameState, RoundNumber);

}

void AZombiesGameState::SetRoundNumber(int32 NewRoundNumber)
{
	if (!HasAuthority() || RoundNumber == NewRoundNumber) return;
	RoundNumber = FMath::Max(1, NewRoundNumber);
	BroadcastRoundNumber();
}

void AZombiesGameState::OnRep_RoundNumber()
{
	BroadcastRoundNumber();
}

void AZombiesGameState::BroadcastRoundNumber()
{
	OnRoundNumberChanged.Broadcast(RoundNumber);
}