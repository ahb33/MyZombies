// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyPlayerState.h"
#include "LobbyGameState.h"
#include "LobbyGameMode.h"
#include "Net/UnrealNetwork.h"

ALobbyPlayerState::ALobbyPlayerState()
    : bReady(false)
{
}
void ALobbyPlayerState::SetReadyStatus(bool bIsReady)
{
    if (HasAuthority())
    {
        UpdateReadyStatus(bIsReady);
    }
    else
    {
        ServerSetReadyStatus(bIsReady);
    }
}


void ALobbyPlayerState::ServerSetReadyStatus_Implementation(bool bIsReady)
{
    UpdateReadyStatus(bIsReady);
}


void ALobbyPlayerState::UpdateReadyStatus(bool bIsReady)
{
    if (bReady == bIsReady) return; // No change

    bReady = bIsReady;

    if (HasAuthority())
    {
        if (auto* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
        {
            GM->CheckLobbyReady();
        }
    }
}

void ALobbyPlayerState::OnRep_Ready()
{
    // e.g. update HUD, play sound, log
    UE_LOG(LogTemp, Log, TEXT("%s is now %s"),
        *GetPlayerName(),
        bReady ? TEXT("Ready") : TEXT("Not Ready"));
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlayerState, bReady);
}
