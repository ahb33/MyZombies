// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
#include "LobbyGameMode.generated.h"

class APlayerController;

UCLASS()
class MYZOMBIES_API ALobbyGameMode : public ABaseGameMode
{
    GENERATED_BODY()
public:
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    
    void StartLobby();
    void CheckLobbyReady();
    void StartGameMatch();

private: 

};
