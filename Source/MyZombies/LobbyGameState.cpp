// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "MyGameInstance.h"
#include "MultiplayerSessions.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"



bool ALobbyGameState::AreAllPlayersReady()
{
    int32 TotalPlayers = PlayerArray.Num();
    int32 ReadyCount = 0;

    UE_LOG(LogTemp, Warning, TEXT("Checking readiness: %d total players."), TotalPlayers);

    for (APlayerState* PS : PlayerArray)
    {
        if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS))
        {
            if (LobbyPS->IsReady())
            {
                ReadyCount++;
                UE_LOG(LogTemp, Warning, TEXT("%s is READY"), *LobbyPS->GetPlayerName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("%s is NOT ready"), *LobbyPS->GetPlayerName());
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Ready players: %d / %d"), ReadyCount, TotalPlayers);

    if (ReadyCount == TotalPlayers && TotalPlayers > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("All players are ready! Starting game..."));
        StartGameMatch();
        return true;
    }

    return false;
}

void ALobbyGameState::StartGameMatch()
{

    UWorld* World = GetWorld();

    UMyGameInstance* GI = World ? World->GetGameInstance<UMyGameInstance>() : nullptr;
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("StartGameMatch: GameInstance not found!"));
        return;
    }

    const FName SelectedMode = GI->GetSelectedGameMode();
    const FString ModeString = SelectedMode.ToString();
    UE_LOG(LogTemp, Warning, TEXT("StartGameMatch: SelectedMode = %s"), *ModeString);

    FString MapPath;
    if (ModeString.Contains("Zombies"))
    {
        MapPath = "/Game/GameAssets/Levels/Zombies_Level?listen";
        UE_LOG(LogTemp, Warning, TEXT("StartGameMatch: Launching Zombies Level"));
    }
    else if (ModeString.Contains("Solo"))
    {
        MapPath = "/Game/GameAssets/Levels/SoloLevel?listen";
        UE_LOG(LogTemp, Warning, TEXT("StartGameMatch: Launching Solo Level"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("StartGameMatch: Unknown mode selected: %s"), *ModeString);
        return;
    }

    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->bUseSeamlessTravel = true;
    }

    World->ServerTravel(MapPath);
}
