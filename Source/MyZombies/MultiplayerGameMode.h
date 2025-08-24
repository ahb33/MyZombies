// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
#include "MultiplayerGameMode.generated.h"

/**
 * 
 */



// I NEED TO RENAME THIS CLASS ! ITS TOO BROADLY NAMED ! 

UCLASS()
class MYZOMBIES_API AMultiplayerGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
    AMultiplayerGameMode();

    UFUNCTION(BlueprintCallable)
    void StartGameWithSelectedMode(FString SelectedMode);

protected:
    virtual void BeginPlay() override;
	
};
