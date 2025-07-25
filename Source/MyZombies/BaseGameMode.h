// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"

UCLASS(minimalapi)
class ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseGameMode();

	virtual void EndGame(bool bPlayerWon);

	// Function to be overridden in child classes for custom logic
	virtual void CheckEnemiesAlive();

	bool bUseSeamlessTravel;

protected:
	virtual void BeginPlay() override;

	// Base function for initializing gameplay (can be overridden)
	virtual void InitializeGameplay();

	UPROPERTY(EditAnywhere, Category = "EndGameWidgets")
	TSubclassOf<class UUserWidget> YouWonWidgetClass;
	
};



