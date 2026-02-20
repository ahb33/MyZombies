// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameState.h"
#include "GameFramework/PlayerStart.h"
#include "BaseGameMode.generated.h"


class AMyPlayerController;


UCLASS(minimalapi)
class ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseGameMode();

	/** Server-only: spawn/possess a pawn for the newly joined player. */
    virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Correct override for AGameModeBase::ChoosePlayerStart (BlueprintNativeEvent). */
	virtual AActor* ChoosePlayerStart_Implementation (AController* Player) override;

	/** Server-only: rule hook. Child classes implement their own logic. */
	virtual void HandlePlayerDeath(AController* Victim, AController* Killer) {}

	/** Server-only: respawn helper (can be overridden). */
	virtual void RequestSpawn(AController* Controller);

protected:
	virtual void BeginPlay() override;

	void SetMatchPhase(enum class EMatchPhase NewPhase);

	ABaseGameState* GetBaseGameState() const;

	// --- Damage policy toggles ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage Rules") 
	bool bAllowPlayerVsPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage Rules") 
	bool bAllowAIVsAI = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage Rules") 
	bool bAllowFriendlyFireWithinTeam = false;

	/** Respawn delay (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawning")
	float RespawnDelay = 4.0f;


private:
	/** Cached PlayerStarts for quick spawn selection. */
	UPROPERTY()
	TArray<TObjectPtr<APlayerStart>> SpawnPoints;

	void DestroyCurrentPawn(AController* Controller);
	void RestartPlayerSafe(AController* Controller);
};




