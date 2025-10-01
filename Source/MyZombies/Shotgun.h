// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h" 
#include "Projectile.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
	
public:


    virtual void Fire(const FVector& Hit) override;

	// modify WaponTrace function from Assault Weapon to include scatter 
	void WeaponTraceWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
    int32 GetNumPellets() const { return NumPellets; }

	virtual float GetDamage() const override;

private:

    UPROPERTY(EditAnywhere, Category="Shotgun")
    int32 NumPellets = 10;

    UPROPERTY(EditAnywhere, Category="Shotgun")
    float ScatterAngle = 10.f; // degrees

};
