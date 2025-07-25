// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h" 
#include "Sound/SoundCue.h"
#include "Projectile.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API AShotgun : public AWeapon
{
	GENERATED_BODY()
	
public:


    virtual void Fire(const FVector& Hit); // Virtual for child class overrides.

	// modify WaponTrace function from Assault Weapon to include scatter 
	void WeaponTraceWithScatter(const FVector& TraceStart, const FVector& HitTarget, FHitResult& FireHit);

	virtual void SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag) override;

    virtual int32 GetCurrentAmmoOnHand() const override;

    virtual int32 GetCurrentAmmoInMag() const override;

	virtual int32 GetMaxAmmoOnHand() const override;

	virtual int32 GetMagCapacity() const override;

	virtual void ReloadAmmo(int32 Ammo) override;

	virtual float GetDamage() const override;

	FVector CalculateScatterEndPoint(const FVector& MuzzleLocation, const FVector& HitTarget);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
    void OnRep_ShotgunAmmoOnHand();

    UFUNCTION()
    void OnRep_ShotgunAmmoInMag();


private:

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ShotgunAmmoOnHand)
    int32 ShotgunAmmoOnHand;

    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ShotgunAmmoInMag)
    int32 ShotgunAmmoInMag;

    UPROPERTY(EditAnywhere, Replicated)
    int32 ShotgunMaxAmmoOnHand;

    UPROPERTY(EditAnywhere, Replicated)
    int32 ShotgunMagCapacity;

};
