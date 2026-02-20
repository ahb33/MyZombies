// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "MyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBlackboardComponent;
class UBehaviorTree;
class UCharacterMovementComponent;
class AMainCharacter;
class AEnemyCharacter;
/*

	This class will be used to control AI controller
 
 */
UCLASS()
class MYZOMBIES_API AMyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AMyAIController();
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus);

	void UpdateNearbyAgents();

	bool IsPlayerWithinRange(const AMainCharacter* Player, const AEnemyCharacter* Enemy) const; // this function will calculate linear distance between character components of AI and maincharacter

private:
	// --- Components / Config
	UPROPERTY(VisibleAnywhere, Category="AI|Perception", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAIPerceptionComponent> MyPerceptionComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig = nullptr;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig = nullptr;

	// Prefer weak ptr to avoid dangling references if player is destroyed/respawned.
	UPROPERTY()
	TWeakObjectPtr<AMainCharacter> MainCharacter;

	// --- Runtime
	UPROPERTY(VisibleAnywhere, Category="AI")
	FVector LastKnownPlayerPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category="AI")
	TArray<FVector> NearbyAgentLocations;

	// Cache default walk speed to restore when not chasing.
	float DefaultWalkSpeed = 0.f;

	// --- Blackboard keys (use FNames to avoid typos)
	static const FName KEY_CanSeePlayer;
	static const FName KEY_CanHearPlayer;
	static const FName KEY_Player;
	static const FName KEY_LastKnownPosition;
	static const FName KEY_PlayerWithinRange;

	// Helpers
	static void SetWalkSpeedIfChanged(UCharacterMovementComponent* MoveComp, float Desired);
};