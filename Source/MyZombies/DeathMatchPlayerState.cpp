// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchPlayerState.h"
#include "Net/UnrealNetwork.h"

#include "MyPlayerController.h"



void ADeathMatchPlayerState::HandlePlayerStatsChanged()
{
    if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwner()))
    {
        PC->UpdateHUDKillDeath(GetPlayerKills(), GetPlayerDeaths());
    }
}