// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"
#include "DeathMatchPlayerState.h"
#include "DeathMatchGameState.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
    PlayerStateClass = ADeathMatchPlayerState::StaticClass();  // per-player stats
    GameStateClass   = ADeathMatchGameState::StaticClass();    // shared match state
}


void ADeathMatchGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("ADeathMatchGameMode started"));
}

void ADeathMatchGameMode::OnMatchStart()
{
    UE_LOG(LogTemp, Warning, TEXT("Match Started!"));
}

void ADeathMatchGameMode::OnMatchEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("Match Ended!"));
}

void ADeathMatchGameMode::OnPlayerKilled(AController *Attacker, AController *Victim)
{
    UE_LOG(LogTemp, Warning, TEXT("Player Killed !"));
}

void ADeathMatchGameMode::RequestSpawn(AController *Victim)
{
    UE_LOG(LogTemp, Warning, TEXT("Requesting Spawn!"));
}
