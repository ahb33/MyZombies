// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainCharacter.h"
#include "TurnInPlace.h"
#include "Weapon.h"
#include "CombatState.h"
#include "MyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override; // this function is called once at the beginning of the game

	UFUNCTION(BlueprintCallable, Category = Animation)
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	void UpdateMovementProperties();
	void UpdateCharacterProperties(float);
	void UpdateWeaponProperties();
	
	
private: 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class AMainCharacter* MainCharacter;

	class AWeapon* playerWeapon;

	// class UCombatComponent* combatComp;

	class APawn* Pawn; // This variable is a reference to the pawn currently using this AnimInstance

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float MovementSpeed; // we only want C++ to alter this variable

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir; // this variable will check if the character is  in the air

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation,meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // this variable will check if the character is accelerating

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bIsPlayerReloading;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bFireWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace; 

	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;


};
