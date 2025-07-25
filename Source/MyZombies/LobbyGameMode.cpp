// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "Kismet/GameplayStatics.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // No need to trigger AreAllPlayersReady here
    UE_LOG(LogTemp, Log, TEXT("LobbyGameMode: Player joined."));
}

void ALobbyGameMode::StartLobby()
{
    bUseSeamlessTravel = true;
    GetWorld()->ServerTravel("/Game/GameAssets/Levels/LobbyLevel?listen");
}