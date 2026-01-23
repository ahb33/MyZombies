// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGameState.h"
#include "Net/UnrealNetwork.h"

void ABaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // DOREPLIFETIME(ABaseGameState, MatchMode);
    DOREPLIFETIME(ABaseGameState, InputProfile);
}

void ABaseGameState::SetInputProfile(EInputProfile InProfile)
{
    if(!HasAuthority()) return;   

    InputProfile = InProfile;
    BroadcastInputProfile();

    
}

void ABaseGameState::BroadcastInputProfile()
{
    OnInputProfileChanged.Broadcast(InputProfile);
}

void ABaseGameState::OnRep_InputProfile()
{
    BroadcastInputProfile();
}



