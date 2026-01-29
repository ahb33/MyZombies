// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"
#include "DeathMatchPlayerState.h"
#include "DeathMatchGameState.h"
#include "MyPlayerController.h"

ADeathMatchGameMode::ADeathMatchGameMode() {}

void ADeathMatchGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ABaseGameState* GS = GetGameState<ABaseGameState>())
	{
		GS->SetInputProfile(EInputProfile::Gameplay);
		GS->SetMatchMode(EMatchMode::Deathmatch);
		GS->SetMatchPhase(EMatchPhase::Intro);
	}

	// Start first round.
	if (ABaseGameState* GS = GetGameState<ABaseGameState>())
	{
		GS->SetMatchPhase(EMatchPhase::Active);
	}
}

void ADeathMatchGameMode::HandlePlayerDeath(AController* Victim, AController* Killer)
{
	EndRound();
	StartNextRoundOrEndMatch();
}

void ADeathMatchGameMode::EndRound()
{
	SetMatchPhase(EMatchPhase::RoundOver);
	// Optional: freeze input, stop scoring, etc. (server-side only).
}

void ADeathMatchGameMode::StartNextRoundOrEndMatch()
{
	if (CurrentRound >= MaxRounds)
	{
		SetMatchPhase(EMatchPhase::GameOver);
		return;
	}

	++CurrentRound;
	SetMatchPhase(EMatchPhase::Intro);
	SetMatchPhase(EMatchPhase::Active);
}