// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MultiplayerSessions.h"
#include "MyGameInstance.generated.h"

/**if (!PC || !PC->IsLocalController()) return;
 * 
 */
UCLASS()
class MYZOMBIES_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

    public:
    UFUNCTION(BlueprintCallable, Category="GameMode")
    void SetSelectedGameMode(FName GameMode) { SelectedGameMode = GameMode; }

    UFUNCTION(BlueprintCallable, Category="GameMode")
    FName GetSelectedGameMode() const { return SelectedGameMode; }

    UFUNCTION(BlueprintPure, Category="Multiplayer")
    UMultiplayerSessions* GetMultiplayerSessions() const
    {
        return GetSubsystem<UMultiplayerSessions>();
    }

private:
    /** Stored as an FName for fast comparisons and level-name lookups */
    UPROPERTY()
    FName SelectedGameMode;

};
