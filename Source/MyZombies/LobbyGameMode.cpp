// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "MyGameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


void ALobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
    {
        LGS->DesiredPlayerCount = UGameplayStatics::GetIntOption(Options, TEXT("desired"), 0);
        UE_LOG(LogTemp, Log, TEXT("Lobby desired players = %d"), LGS->DesiredPlayerCount);
    }
}

//  runs once when the lobby map loads (server)
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("Player joined. Now in lobby."));
    CheckLobbyReady(); // re-evaluate on join
}


void ALobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    UE_LOG(LogTemp, Log, TEXT("Player left. Re-checking lobby."));
    CheckLobbyReady(); // re-evaluate on leave
}


void ALobbyGameMode::StartLobby()
{
    bUseSeamlessTravel = true;
    GetWorld()->ServerTravel("/Game/GameAssets/Levels/LobbyLevel?listen");
}

void ALobbyGameMode::CheckLobbyReady()
{
    if (const auto* LGS = GetGameState<ALobbyGameState>())
    {
        if (LGS->AreAllPlayersReady())
        {
            UE_LOG(LogTemp, Log, TEXT("Everyone ready – starting via GameState"));
            StartGameMatch();
        }
    }
}

void ALobbyGameMode::StartGameMatch()
{
    UWorld* World = GetWorld();

    UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();

    const FString Mode = GI->GetSelectedGameMode().ToString();
    
    FString MapPath;
    if (Mode.Contains("Zombies"))
        MapPath = "/Game/GameAssets/Levels/Zombies_Level?listen";
    else if (Mode.Contains("DeathMatch"))
        MapPath = "/Game/GameAssets/Levels/DeathMatch_Level?listen";
        
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown mode: %s"), *Mode);
        return;
    }

    bUseSeamlessTravel = true;
    World->ServerTravel(MapPath);
}
