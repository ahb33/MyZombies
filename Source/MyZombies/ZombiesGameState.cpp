// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombiesGameState.h"
#include "Net/UnrealNetwork.h"


void AZombiesGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AZombiesGameState, RoundNumber);
    DOREPLIFETIME(AZombiesGameState, RoundPhase);

}
