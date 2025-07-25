// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Projectile.h"
#include "AssaultWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API AAssaultWeapon : public AWeapon
{
	GENERATED_BODY()

public:

    virtual void Fire(const FVector& Hit); // Virtual for child class overrides.

	virtual void SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag) override;

    virtual int32 GetCurrentAmmoOnHand() const override;

    virtual int32 GetCurrentAmmoInMag() const override;

	virtual int32 GetMaxAmmoOnHand() const override;

	virtual int32 GetMagCapacity() const override;

	virtual void ReloadAmmo(int32 Ammo) override;

	virtual float GetDamage() const override {return Damage;}
private:

    UPROPERTY(EditAnywhere)
	int32 AssaultWeaponAmmoOnHand; // 

    UPROPERTY(EditAnywhere)
	int32 AssaultWeaponAmmoInMag; // 

    UPROPERTY(EditAnywhere)
    int32 AssaultWeaponMaxAmmoOnHand;

    UPROPERTY(EditAnywhere)
    int32 AssaultWeaponMagCapacity;

	FTransform SocketTransform;
};
