// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "WeaponTypes.h"
#include "AmmoPickUp.generated.h"

/**
 * 
 */

class AMainCharacter;  // ✅ Forward declaration
class AWeapon;      
	
UCLASS()
class MYZOMBIES_API AAmmoPickUp : public APickUp
{
	GENERATED_BODY()


public:

	EWeaponType GetWeaponType() const {return WeaponType;}



protected: 

	
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// you only want overlap events to happen on the server


private:

	AMainCharacter* MainCharacter;  // ✅ Correctly inside class now
	AWeapon* myWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	int32 AmmoAmount;

	int32 GetAmmoAmountForWeaponType( );
};

