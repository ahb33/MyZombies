// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

    // Firing
    virtual void Fire(const FVector& Hit);

	virtual float GetDamage() const override {return Damage;}


protected:
	bool HitScanTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const;

};
