// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Projectile.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:

    virtual void Fire(const FVector& Hit); // Virtual for child class overrides.

	virtual float GetDamage() const override {return Damage;}
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
    
	FTransform SocketTransform;
};
