// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"
#include "MainCharacter.h"
#include "Weapon.h"

AAmmoPickUp::AAmmoPickUp()
{
	if (WeaponType == EWeaponType::None)
	{
		WeaponType = EWeaponType::AssaultRifle;
	}

	if (AmmoAmount <= 0)
	{
		AmmoAmount = GetAmmoAmountForWeaponType(WeaponType);
	}
}

int32 AAmmoPickUp::GetAmmoAmountForWeaponType(EWeaponType InType)
{
	switch (InType)
	{
	case EWeaponType::AssaultRifle: return 50;
	case EWeaponType::Shotgun:      return 8;
	default:                        return 0;
	}
}

bool AAmmoPickUp::TryConsume(APawn* ByPawn)
{
	if (!HasAuthority()) return false;
	AMainCharacter* MC = Cast<AMainCharacter>(ByPawn);
	if (!MC) return false;

	if (AmmoAmount <= 0) AmmoAmount = GetAmmoAmountForWeaponType(WeaponType);

	const bool bAdded = MC->AddAmmoFromPickup(WeaponType, AmmoAmount); 
	if (!bAdded) return false;

	Destroy();
	return true;
}