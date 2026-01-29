// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BaseGameState.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
    Intro,
    Active,
    RoundOver,
    GameOver
};


UENUM(BlueprintType)
enum class EInputProfile : uint8
{
  Lobby,
  Gameplay
};


UENUM(BlueprintType) 
enum class EMatchMode : uint8 
{ 
  Zombies, 
  Deathmatch
};



DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EMatchPhase);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputProfileChanged, EInputProfile);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchModeChanged, EMatchMode);

UCLASS()
class MYZOMBIES_API ABaseGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	FOnMatchPhaseChanged OnMatchPhaseChanged;
	FOnInputProfileChanged OnInputProfileChanged;
	FOnMatchModeChanged OnMatchModeChanged;

	UFUNCTION(BlueprintPure, Category="Match")
	EMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category="Match")
	EMatchMode GetMatchMode() const { return MatchMode; }

	UFUNCTION(BlueprintPure, Category="Input")
	EInputProfile GetInputProfile() const { return InputProfile; }

	/** Server-only setters (GameMode should call these). */
	UFUNCTION(BlueprintCallable, Category="Match")
	void SetMatchPhase(EMatchPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category="Match")
	void SetMatchMode(EMatchMode NewMode);

	UFUNCTION(BlueprintCallable, Category="Input")
	void SetInputProfile(EInputProfile NewProfile);


protected:
  virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(ReplicatedUsing=OnRep_MatchMode)
  EMatchMode MatchMode = EMatchMode::Zombies; // set default

  UPROPERTY(ReplicatedUsing=OnRep_InputProfile)
  EInputProfile InputProfile = EInputProfile::Lobby; // set default

  UPROPERTY(ReplicatedUsing=OnRep_MatchPhase)
  EMatchPhase MatchPhase = EMatchPhase::Intro; // set default

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_MatchMode();

	UFUNCTION()
	void OnRep_InputProfile();

private:
	void BroadcastMatchPhase();
	void BroadcastMatchMode();
	void BroadcastInputProfile();
	
};
