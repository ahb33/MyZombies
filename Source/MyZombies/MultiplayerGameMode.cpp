// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGameMode.h"
#include "MultiplayerGameMode.h"
#include "MultiplayerGameState.h" 
#include "Kismet/GameplayStatics.h"


AMultiplayerGameMode::AMultiplayerGameMode() {}

void AMultiplayerGameMode::BeginPlay()
{
    Super::BeginPlay();

    GameStateClass = AMultiplayerGameState::StaticClass();

    UE_LOG(LogTemp, Warning, TEXT("MultiplayerGameMode started"));
}

void AMultiplayerGameMode::StartGameWithSelectedMode(FString SelectedMode)
{
    if (SelectedMode == "Zombies")
    {
        UGameplayStatics::OpenLevel(this, FName("SpaceShipLevel_Solo"));
    }
    else if (SelectedMode == "Deathmatch")
    {
        UGameplayStatics::OpenLevel(this, FName("DeathmatchLevel"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown game mode selected: %s"), *SelectedMode);
    }
}

