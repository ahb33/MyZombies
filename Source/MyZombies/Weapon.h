// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponState.h"      // uses EWeaponState in UPROPERTY
#include "WeaponTypes.h"      // uses EWeaponType in UPROPERTY
#include "Weapon.generated.h"

// Forward declarations (avoid heavy includes in header)
class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UParticleSystem;
class USoundCue;
class AMainCharacter;
class AProjectile;
class ACasing;

constexpr float TRACE_LENGTH = 10000.f;

UCLASS()
class MYZOMBIES_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();
    virtual void Tick(float DeltaTime) override;

    friend class AMainCharacter;

    // UI
    void ShowPickUpWidget(bool bShowWidget);
    void DropWeapon();

    // HUD
    void RefreshHUD();
    void SetHUDAmmo(int32 Ammo);
    void SetHUDMagAmmo(int32 MagAmmo);

    // Firing
    virtual void Fire(const FVector& Hit);
    void RoundFired();

    // Ammo / Damage
    void SetAmmoInMag();
    void SetAmmoOnHand();
    virtual void SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag)
    {
        AmmoOnHand = NewAmmoOnHand;
        AmmoInMag  = NewAmmoInMag;
    }
    virtual int32  GetCurrentAmmoOnHand() const { return AmmoOnHand; }
    virtual int32  GetCurrentAmmoInMag()  const { return AmmoInMag; }
    virtual int32  GetMaxAmmoOnHand()     const { return MaxAmmoOnHand; }
    virtual int32  GetMagCapacity()       const { return MagCapacity; }
    virtual void   ReloadAmmo(int32 AmmoToAdd)
    {
        const int32 AmmoToReload = FMath::Min(AmmoToAdd, MagCapacity - AmmoInMag);
        AmmoInMag  += AmmoToReload;
        AmmoOnHand -= AmmoToReload;
    }
    virtual float  GetDamage() const { return Damage; }
    void           DealDamage(const FHitResult& HitResult);

    // Utilities
    bool WeaponIsEmpty() const;
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    void         SetWeaponState(EWeaponState NewState);
    EWeaponType  GetWeaponType()  const { return WeaponType; }
    EWeaponState GetWeaponState() const { return WeaponState; }
    virtual void SetOwner(AActor* NewOwner) override;
    UParticleSystem* GetTracer()         const { return Tracer; }
    UParticleSystem* GetImpactParticles()const { return ImpactParticles; }
    USoundCue*       GetImpactSound()    const { return ImpactSound; }
    FORCEINLINE float GetZoomedFOV()     const { return ZoomedFOV; }
    FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner();
    UFUNCTION() void OnRep_AmmoOnHand();
    UFUNCTION() void OnRep_AmmoInMag();
    UFUNCTION() void OnRep_WeaponState();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnSphereOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnSphereEndOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Weapon, meta=(AllowPrivateAccess="true"))
    TSubclassOf<AProjectile> Projectile;

    UPROPERTY(EditAnywhere) UParticleSystem* MuzzleFlash = nullptr;
    UPROPERTY(EditAnywhere) float Damage = 0.f;

private:
    // Components
    UPROPERTY(VisibleAnywhere, Category=Weapon)
    USphereComponent* AreaSphere = nullptr;

    UPROPERTY(VisibleAnywhere, Category=Weapon)
    UWidgetComponent* PickupWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon, meta=(AllowPrivateAccess="true"))
    USkeletalMeshComponent* WeaponMesh = nullptr;

    // FX
    UPROPERTY(EditAnywhere, Category=Projectile) UParticleSystem* Tracer = nullptr;
    UPROPERTY(EditAnywhere, Category=Projectile) UParticleSystem* ImpactParticles = nullptr;
    UPROPERTY(EditAnywhere) USoundCue* ImpactSound = nullptr;

    // Animation
    UPROPERTY(EditAnywhere, Category=WeaponAnimation)
    UAnimationAsset* FireAnim = nullptr;

    // Owner refs
    UPROPERTY() AMainCharacter* MainCharacter = nullptr;

    // Ammo/mags
    UPROPERTY(ReplicatedUsing=OnRep_AmmoOnHand) int32 AmmoOnHand = 0;
    UPROPERTY(ReplicatedUsing=OnRep_AmmoOnHand) int32 MaxAmmoOnHand = 0;   // consider plain Replicated if no notify needed
    UPROPERTY(ReplicatedUsing=OnRep_AmmoOnHand) int32 ReloadAmount = 0;    // consider plain Replicated if no notify needed
    UPROPERTY(ReplicatedUsing=OnRep_AmmoInMag)  int32 AmmoInMag = 0;

    UPROPERTY() int32 MagCapacity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Combat", meta=(AllowPrivateAccess="true"))
    EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle; // example default

    UPROPERTY(ReplicatedUsing=OnRep_WeaponState)
    EWeaponState WeaponState = EWeaponState::EWS_Unequipped;

    // Other
    UPROPERTY(EditAnywhere, Category=Casing) TSubclassOf<ACasing> Casing;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomedFOV = 35.f;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomInterpSpeed = 15.f;
};