// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"
#include "MainCharacter.h"
#include "Weapon.h"


bool AAmmoPickUp::TryConsume(APawn* ByPawn)
{
	if (!HasAuthority()) return false;
	AMainCharacter* MC = Cast<AMainCharacter>(ByPawn);
	if (!MC) return false;

	if (AmmoAmount <= 0) AmmoAmount = GetAmmoAmountForWeaponType();

	const bool bAdded = MC->AddAmmoFromPickup(WeaponType, AmmoAmount); 
	if (!bAdded) return false;

	Destroy();
	return true;
}