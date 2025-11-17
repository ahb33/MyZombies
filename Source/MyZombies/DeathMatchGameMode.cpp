// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"
#include "DeathMatchPlayerState.h"
#include "DeathMatchGameState.h"
#include "MyPlayerController.h"

ADeathMatchGameMode::ADeathMatchGameMode()
{
    PlayerStateClass = ADeathMatchPlayerState::StaticClass();  // per-player stats
    GameStateClass   = ADeathMatchGameState::StaticClass();    // shared match state
    GameStateClass = ABaseGameState::StaticClass();
}


void ADeathMatchGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (auto* GS = GetGameState<ABaseGameState>()) GS->SetMatchMode(EMatchMode::Deathmatch);
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
    if (Attacker && Attacker->PlayerState)
    {
        if (ADeathMatchPlayerState* AttackerPS = Cast<ADeathMatchPlayerState>(Attacker->PlayerState))
        {
            AttackerPS->AddKill();

            /*
                •	If AttackerPS->GetKills() >= ScoreToWin:
                o	Set GameState->MatchPhase = EMatchPhase::EMP_PostMatch;
                o	Call OnMatchEnd().

            */
            if(AttackerPS->GetPlayerKills() >= ScoreToWin)
            {
                if(ADeathMatchGameState* DMGameState = GetGameState<ADeathMatchGameState>())
                DMGameState->MatchPhase = EMatchPhase::PostMatch;
                OnMatchEnd();
            }
        }
    }
    
    if (Victim && Victim->PlayerState)
    {
        if (ADeathMatchPlayerState* VictimPS = Cast<ADeathMatchPlayerState>(Victim->PlayerState))
        {
            VictimPS->AddDeath();
        }
    }
}

void ADeathMatchGameMode::RequestSpawn(AController *Victim)
{
    UE_LOG(LogTemp, Warning, TEXT("Requesting Spawn!"));
}
