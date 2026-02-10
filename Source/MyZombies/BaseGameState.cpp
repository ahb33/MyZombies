// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGameState.h"
#include "Net/UnrealNetwork.h"

void ABaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseGameState, MatchPhase);
	DOREPLIFETIME(ABaseGameState, MatchMode);
	DOREPLIFETIME(ABaseGameState, InputProfile);
}

void ABaseGameState::SetMatchPhase(EMatchPhase NewPhase)
{
	if (!HasAuthority() || MatchPhase == NewPhase) return;
	MatchPhase = NewPhase;
	ForceNetUpdate();
	BroadcastMatchPhase();
}

void ABaseGameState::SetMatchMode(EMatchMode NewMode)
{
	if (!HasAuthority() || MatchMode == NewMode) return;
	MatchMode = NewMode;
	BroadcastMatchMode();
}

void ABaseGameState::SetInputProfile(EInputProfile NewProfile)
{
	if (!HasAuthority() || InputProfile == NewProfile) return;
	InputProfile = NewProfile;
	BroadcastInputProfile();
}

void ABaseGameState::OnRep_MatchPhase()
{
	BroadcastMatchPhase();
}

void ABaseGameState::OnRep_MatchMode()
{
	BroadcastMatchMode();
}

void ABaseGameState::OnRep_InputProfile()
{
	BroadcastInputProfile();
}

void ABaseGameState::BroadcastMatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void ABaseGameState::BroadcastMatchMode()
{
	OnMatchModeChanged.Broadcast(MatchMode);
}

void ABaseGameState::BroadcastInputProfile()
{
	OnInputProfileChanged.Broadcast(InputProfile);
}



