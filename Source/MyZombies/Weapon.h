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

#ifndef TRACE_LENGTH
#define TRACE_LENGTH 80000.f
#endif


static const FName MUZZLE_NAME(TEXT("MuzzleFlash"));

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
    float GetFireDelay() const { return FireDelay; }
    UFUNCTION()
    virtual void PlayFireEffects(const FHitResult& Hit, const FVector& Start, const FVector& End);
    void UpdateTracer(const FVector& SourceWS, const FVector& TargetWS);

    // Ammo / Damage
    void SetAmmoInMag();
    void SetAmmoOnHand();
    virtual void ReloadAmmo(int32 RequestedAmmo);
    virtual void SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag)
    {
        AmmoOnHand = NewAmmoOnHand;
        AmmoInMag  = NewAmmoInMag;
    }
    virtual int32  GetCurrentAmmoOnHand() const { return AmmoOnHand; }
    virtual int32  GetCurrentAmmoInMag()  const { return AmmoInMag; }
    virtual int32  GetMaxAmmoOnHand()     const { return MaxAmmoOnHand; }
    virtual int32  GetMagCapacity()       const { return MagCapacity; }

    virtual float  GetDamage() const { return Damage; }
    void DealDamage(const FHitResult& HitResult);

    // Utilities
    bool WeaponIsEmpty() const;
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    void SetWeaponState(EWeaponState NewState);
    void ApplyWeaponState();
    EWeaponType  GetWeaponType()  const { return WeaponType; }
    EWeaponState GetWeaponState() const { return WeaponState; }
    virtual void SetOwner(AActor* NewOwner) override;
    UParticleSystem* GetTracer()         const { return Tracer; }
    UParticleSystem* GetImpactParticles() const { return ImpactParticles; }
    UParticleSystem* GetMuzzleFlash() const {return MuzzleFlash;}
    USoundCue*       GetImpactSound()    const { return ImpactSound; }
    FORCEINLINE float GetZoomedFOV()     const { return ZoomedFOV; }
    FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

    static FORCEINLINE const FName& GetMuzzleSocketName()
    {
        static const FName Name(TEXT("MuzzleFlash"));
        return Name;
    }

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner();
    UFUNCTION() void OnRep_AmmoOnHand();
    UFUNCTION() void OnRep_AmmoInMag();
    UFUNCTION() void OnRep_WeaponState();


    /** Accessor for consistent usage. */
    FORCEINLINE float GetMaxTraceDistance() const { return MaxTraceDistance; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Weapon, meta=(AllowPrivateAccess="true"))
    // TSubclassOf<AProjectile> Projectile;

    UPROPERTY(EditAnywhere) float Damage = 0.f;
    
    bool bUseServerSideRewind = false;

    // Ammo/mags
    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_AmmoOnHand, Category = Ammo) int32 AmmoOnHand = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_AmmoInMag, Category = Ammo)  int32 AmmoInMag = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo) int32 MagCapacity = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo) int32 MaxAmmoOnHand = 0;

    UPROPERTY(EditAnywhere, Category="Weapon") float FireDelay = 0.1f;
     
    /** In cm. Default = 800m. Exposed so designers can tune per-weapon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Range", meta=(ClampMin="1.0"))
    float MaxTraceDistance;

private:
    // Components
    UPROPERTY(VisibleAnywhere, Category=Weapon)
    USphereComponent* AreaSphere = nullptr;

    UPROPERTY(VisibleAnywhere, Category=Weapon)
    UWidgetComponent* PickupWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Weapon, meta=(AllowPrivateAccess="true"))
    USkeletalMeshComponent* WeaponMesh = nullptr;

    // FX
    UPROPERTY(EditAnywhere, Category = Weapon) UParticleSystem* MuzzleFlash = nullptr;
    UPROPERTY(EditAnywhere, Category = Weapon) UParticleSystem* ImpactParticles = nullptr;
    UPROPERTY(EditAnywhere, Category = Weapon) USoundCue* ImpactSound = nullptr;
    UPROPERTY(EditAnywhere, Category = Weapon) UParticleSystem* Tracer = nullptr;
    UPROPERTY(EditAnywhere, Category = Weapon) UParticleSystemComponent* TracerSystem = nullptr;
    UPROPERTY(EditAnywhere, Category = Weapon) int32 TracerEmitterIndex = 0;


    // Animation
    UPROPERTY(EditAnywhere, Category=WeaponAnimation)
    UAnimationAsset* FireAnim = nullptr;

    // Owner refs
    UPROPERTY() AMainCharacter* MainCharacter = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Combat", meta=(AllowPrivateAccess="true"))
    EWeaponType WeaponType = EWeaponType::AssaultRifle;  // example default

    UPROPERTY(ReplicatedUsing=OnRep_WeaponState)
    EWeaponState WeaponState = EWeaponState::Unequipped;

    // Other
    UPROPERTY(EditAnywhere, Category=Casing) TSubclassOf<ACasing> Casing;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomedFOV = 35.f;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomInterpSpeed = 15.f;


};