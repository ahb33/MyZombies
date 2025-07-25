// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimationAsset.h" 
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "WeaponState.h"
#include "WeaponTypes.h"
#include "MyHUD.h"
#include "Particles/ParticleSystem.h" 
#include "Sound/SoundCue.h"
#include "Delegates/DelegateCombinations.h"
#include "Casing.h"
#include "Weapon.generated.h"




constexpr float TRACE_LENGTH = 10000.f;

UCLASS()
class MYZOMBIES_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
    /** Constructor and Lifecycle */
    AWeapon();  // Sets default values for this actor's properties.
    virtual void Tick(float DeltaTime) override;  // Called every frame.

    /** Friend Class Access */
    friend class AMainCharacter; // Allows AMainCharacter to access private/protected members.

    /** UI Interaction */
    void ShowPickUpWidget(bool bShowWidget);

    void DropWeapon();

    /** HUD and Crosshair */
    void RefreshHUD();
    void SetHUDAmmo(int32 Ammo);
    void SetHUDMagAmmo(int32 MagAmmo);

    
    /** Weapon Firing */
    virtual void Fire(const FVector& Hit); // Virtual for child class overrides.
    void RoundFired();

    /** Ammo and Damage */
    void SetAmmoInMag();
    void SetAmmoOnHand();
    virtual void SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag)
    {
        AmmoOnHand = NewAmmoOnHand;
        AmmoInMag = NewAmmoInMag;
    }
    virtual int32 GetCurrentAmmoOnHand() const { return AmmoOnHand; }
    virtual int32 GetCurrentAmmoInMag() const { return AmmoInMag; }
    virtual int32 GetMaxAmmoOnHand() const { return MaxAmmoOnHand; }
    virtual int32 GetMagCapacity() const { return MagCapacity; }

    // Ammo Setters
    virtual void ReloadAmmo(int32 AmmoToAdd)
    {
        int32 AmmoToReload = FMath::Min(AmmoToAdd, MagCapacity - AmmoInMag);
        AmmoInMag += AmmoToReload;
        AmmoOnHand -= AmmoToReload;
    }

    virtual float GetDamage() const { return Damage; }
    void DealDamage(const FHitResult& HitResult);

    /** Utilities */
    bool WeaponIsEmpty() const;
	// Useful for accessing the actual mesh of the weapon itself
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
    void SetWeaponState(EWeaponState NewState);
    EWeaponType GetWeaponType() const {return WeaponType;}
    EWeaponState GetWeaponState() const { return WeaponState; }
    virtual void SetOwner(AActor* NewOwner) override;
    UParticleSystem* GetTracer() const { return Tracer; }
    UParticleSystem* GetImpactParticles() const { return ImpactParticles; }
    USoundCue* GetImpactSound() const { return ImpactSound; }
    
    FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
    FORCEINLINE float GetZoomInterpSpeed() const {return ZoomInterpSpeed;}


    /** Replication */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Owner();
    UFUNCTION()
    void OnRep_AmmoOnHand();
    UFUNCTION()
    void OnRep_AmmoInMag();
    UFUNCTION()
    void OnRep_WeaponState();





protected:

    /** Lifecycle */
    virtual void BeginPlay() override;

    /** Overlap Events */
    UFUNCTION()
    virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AProjectile> Projectile; /*we can populate this variable with projectile or anything derived of  projectile*/

    UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

    UPROPERTY(EditAnywhere)
	float Damage;





private:

    /** Core Components */
    UPROPERTY(VisibleAnywhere, Category = Weapon)
    class USphereComponent* AreaSphere;

    UPROPERTY(VisibleAnywhere, Category = Weapon)
    class UWidgetComponent* PickupWidget;
    
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(EditAnywhere, Category = Projectile)
    class UParticleSystem* Tracer;

    UPROPERTY(EditAnywhere, Category = Projectile)
    class UParticleSystem* ImpactParticles;

    UPROPERTY(EditAnywhere)
    USoundCue* ImpactSound;

    /** Animation */
    UPROPERTY(EditAnywhere, Category = WeaponAnimation)
    UAnimationAsset* FireAnim;

    /** Player and Character References */
    UPROPERTY()
    class AMainCharacter* MainCharacter; // Access to MainCharacter functions.

    UPROPERTY(ReplicatedUsing = OnRep_AmmoOnHand)
    int32 AmmoOnHand; // Current ammo available for the weapon.

    UPROPERTY(ReplicatedUsing = OnRep_AmmoOnHand)
    int32 MaxAmmoOnHand; // Maximum ammo capacity for the weapon.

    UPROPERTY(ReplicatedUsing = OnRep_AmmoOnHand)
    int32 ReloadAmount; // Ammo reloaded per action.

    UPROPERTY(ReplicatedUsing = OnRep_AmmoInMag)
    int32 AmmoInMag; // Ammo in the magazine.

	/** Ammo and Capacity */
    UPROPERTY()
    int32 MagCapacity; // Maximum magazine capacity.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Combat", meta = (AllowPrivateAccess = "true")) 
    EWeaponType WeaponType;

    UPROPERTY(ReplicatedUsing = OnRep_WeaponState)
    EWeaponState WeaponState = EWeaponState::EWS_Unequipped;

    /** Casing and Effects */
    UPROPERTY(EditAnywhere, Category = Casing)
    TSubclassOf<class ACasing> Casing; // Ejectable shell casing class

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ZoomedFOV = 35.0f; // will change based on weapon

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ZoomInterpSpeed = 15.0f; // will change based on weapon




};
