// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BaseGameState.generated.h"

/**
 * 
 */

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


DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputProfileChanged, EInputProfile);


UCLASS()
class MYZOMBIES_API ABaseGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

  FOnInputProfileChanged OnInputProfileChanged; 

  UFUNCTION() 
  EMatchMode GetMatchMode() const { return MatchMode; }

  UFUNCTION()
  EInputProfile GetInputProfile() const { return InputProfile;}

  UFUNCTION()
  void SetMatchMode(EMatchMode InMode) { if (HasAuthority()) { MatchMode = InMode; } }

  UFUNCTION()
  void SetInputProfile(EInputProfile InputProfile);


protected:
  virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(Replicated, BlueprintReadOnly, Category="Match")
  EMatchMode MatchMode = EMatchMode::Zombies; // set default

  UPROPERTY(ReplicatedUsing=OnRep_InputProfile, BlueprintReadOnly, Category="Input")
  EInputProfile InputProfile = EInputProfile::Lobby; // set default

  UFUNCTION() void OnRep_InputProfile();

private:

  void BroadcastInputProfile();
	
};
