// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	bool AreAllPlayersReady() const;

	void StartGameMatch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby")
    int32 DesiredPlayerCount = 0;

protected:


};