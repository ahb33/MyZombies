// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AI_AnimInstance.generated.h"

/**
 * 
 */

class APawn;
class AEnemyCharacter;  

UCLASS()
class MYZOMBIES_API UAI_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:

	virtual void NativeInitializeAnimation() override; // this function is called once at the beginning of the game

	UFUNCTION(BlueprintCallable, Category = Animation)
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	void UpdatePawnReferences();
	void UpdateMovementSpeed(float DeltaTime);
    void UpdatePerceptionState();


	// Setter function to set the random dying animation variable
	UFUNCTION(BlueprintCallable, Category = "Animation")
	FORCEINLINE void SetIsDead(bool bDead) { bIsDead = bDead; }


	// Setter function to set the random dying animation variable
	UFUNCTION(BlueprintCallable, Category = "Animation")
	FORCEINLINE void SetRandomDyingAnimation(bool bShouldRandomize) { bRandomDyingAnimation = bShouldRandomize; }
	
	// Setter function to set the random dying animation variable
	UFUNCTION(BlueprintCallable, Category = "Animation")
	FORCEINLINE void SetPlayerVisibility(bool bVisible) { bPlayerVisible = bVisible; }

	// Setter function to set the random dying animation variable
	UFUNCTION(BlueprintCallable, Category = "Animation")
	FORCEINLINE void SetPlayerAttackRange(bool bWithinRange) { bPlayerWithinRange = bWithinRange; }


private: 

	TObjectPtr<APawn> PawnInstance = nullptr;
	TObjectPtr<AEnemyCharacter> EnemyCharacterInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float MovementSpeed; // we only want C++ to alter this variable

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // this variable will check if the character is accelerating

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bPlayerVisible; // this variable will check if the zombie can see the player

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bPlayerWithinRange; // this variable will check if the zombie can see the player

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bIsDead; // this variable will check if the zombie can see the player

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bRandomDyingAnimation; // this variable will select between 2 of the dying animations

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	float SpeedInterpRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	float MaxBlendSpeed = 360.f; 
	
	FVector PrevLoc = FVector::ZeroVector;
	bool bHasPrevLoc = false;


};
