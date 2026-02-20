// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameState.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */

class UMultiplayerSessions;

UCLASS()
class MYZOMBIES_API ALobbyGameState : public ABaseGameState
{
	GENERATED_BODY()

public:
    virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	bool AreAllPlayersReady() const;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby")
    int32 DesiredPlayerCount = 0;

protected:

    UPROPERTY()
    TObjectPtr<UMultiplayerSessions> MultiplayerSubsystem = nullptr;

};