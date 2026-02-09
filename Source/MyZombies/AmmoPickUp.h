// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "PickUp.h"
#include "WeaponTypes.h"
#include "AmmoPickUp.generated.h"

class AMainCharacter;
/**
 * 
 */


UCLASS()
class MYZOMBIES_API AAmmoPickUp : public APickUp
{
	GENERATED_BODY()


public:

	AAmmoPickUp();

	EWeaponType GetWeaponType() const {return WeaponType;}

	virtual bool TryConsume(APawn* ByPawn) override;

private:

	UPROPERTY(EditAnywhere, Category="Weapon")
	EWeaponType WeaponType = EWeaponType::AssaultRifle;

	UPROPERTY(EditAnywhere, Category="Weapon", meta=(ClampMin="0"))
	int32 AmmoAmount = 0;

	static int32 GetAmmoAmountForWeaponType(EWeaponType InType);
};

