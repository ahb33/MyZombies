// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include "LobbyPlayerState.h"
#include "MyGameInstance.h"
#include "MultiplayerSessions.h"
#include <algorithm>



// check if all players in lobby are ready ()
// check if all # of players created in UCreateSessionMenuWidget ; already have NumPlayers variable in CreateSessionMenu 

void ALobbyGameState::BeginPlay()
{
    Super::BeginPlay();

    if (const UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>())
    {
        MultiplayerSubsystem = GI->GetMultiplayerSessions();
        if (MultiplayerSubsystem)
        {
            DesiredPlayerCount = MultiplayerSubsystem->GetNumPublicConnections(); 
            UE_LOG(LogTemp, Warning, TEXT("DesiredPlayerCount is %d"), DesiredPlayerCount)
        }
    }
}

bool ALobbyGameState::AreAllPlayersReady() const
{
    if (DesiredPlayerCount <= 0) return false;                 
    if (PlayerArray.Num() != DesiredPlayerCount) return false;  

    return std::all_of(PlayerArray.begin(), PlayerArray.end(),
    [](const APlayerState* PS)
    {
        const auto* LPS = Cast<ALobbyPlayerState>(PS);
        return LPS && LPS->IsReady();

    });
}

