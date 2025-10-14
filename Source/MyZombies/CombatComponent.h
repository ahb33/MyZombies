// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatState.h"          // UENUM used in UPROPERTY
#include "MyHUD.h"                // needed because FHUDPackage is a by-value member
#include "Weapon.h"
#include "CombatComponent.generated.h"

// Forward declarations
class AMainCharacter;
class AMyPlayerController;
class AWeapon;
class UTexture2D;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYZOMBIES_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UCombatComponent();
    friend class AMainCharacter;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void FireButtonPressed(bool bPressed);
    void Fire();
    void FireHitScanWeapon();
    void FireShotgun(); 
    void HitScanLocalFire(const FVector_NetQuantize& TraceHitTarget); // FVector_NetQuantize inherits from FVector
    void ShotgunLocalFire(const FVector_NetQuantize& TraceHitTarget); // FVector_NetQuantize inherits from FVector

    UFUNCTION(NetMulticast, Reliable) 
    void MulticastFireHitScan(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable) 
    void MulticastFireShotgun(const FVector_NetQuantize& TraceHitTarget);

    void EquipWeapon(AWeapon* WeaponToEquip);

    UFUNCTION(Server, Reliable)
    void ServerEquipWeapon(AWeapon* WeaponToEquip);
    void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
    void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
    
    void SwapWeapons();

    void Reload();
    void FinishReloading();
    int32 AmmoToReload() const;
    virtual void ReloadAmmo(int32 Ammo) {}
    void OnFireCooldownFinished();

    void AttachActorToRightHand(AActor* ActorToAttach);
    void AttachWeaponToBackSocket(AActor* WeaponToAttach);

    FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
    FORCEINLINE AWeapon* GetSecondaryWeapon() const {return SecondaryWeapon;}
    FORCEINLINE AMainCharacter* GetMainCharacter() const {return MainCharacter;}

    void SetAiming(bool bIsAiming);
    bool ShouldSwapWeapons() const;
    void SetCombatState(ECombatState State);
    FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

    void TraceUnderCrosshairs(FHitResult& HitResult) const;
    void HandleZoom(float DeltaTime);
    void SetZooming(bool bIsZooming);
    FORCEINLINE bool IsZooming() const { return bZooming; }

    FVector_NetQuantize BuildNetHitTargetFromController() const;

    UFUNCTION() 
    void OnRep_CombatState();
    void SetHUDCrosshairs(float DeltaTime);

    UFUNCTION(Server, Reliable) void ServerReload();
    UFUNCTION(Server, Reliable) void ServerSetAiming(bool bIsAiming);
    UFUNCTION(Server, Reliable) void ServerHitScanFire(const FVector_NetQuantize& TraceHitTarget);
    UFUNCTION(Server, Reliable) void ServerShotgunFire(const FVector_NetQuantize& TraceHitTarget);
    UFUNCTION() void OnRep_EquippedWeapon();
    UFUNCTION() void OnRep_SecondaryWeapon();     


protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
    AWeapon* EquippedWeapon = nullptr;

    UPROPERTY(ReplicatedUsing=OnRep_SecondaryWeapon)
    AWeapon* SecondaryWeapon = nullptr;

    UPROPERTY(ReplicatedUsing= OnRep_CombatState)
    ECombatState CombatState = ECombatState::ECS_Unoccupied;

    // Local refs
    AMainCharacter*      MainCharacter = nullptr;
    AMyPlayerController* Controller     = nullptr;

    UPROPERTY() AMyHUD* HUD = nullptr;

    // HUD data (requires MyHUD.h here because it's by value)
    FHUDPackage HUDPackage{};
    float CrosshairVelocityFactor = 0.f;

    // Crosshairs
    UPROPERTY(EditAnywhere, Category=Crosshairs) UTexture2D* CrosshairsCenter = nullptr;
    UPROPERTY(EditAnywhere, Category=Crosshairs) UTexture2D* CrosshairsLeft   = nullptr;
    UPROPERTY(EditAnywhere, Category=Crosshairs) UTexture2D* CrosshairsRight  = nullptr;
    UPROPERTY(EditAnywhere, Category=Crosshairs) UTexture2D* CrosshairsTop    = nullptr;
    UPROPERTY(EditAnywhere, Category=Crosshairs) UTexture2D* CrosshairsBottom = nullptr;

    // Aiming/zoom
    UPROPERTY(Replicated) 
    bool bAiming = false;
    bool bCanFire = true;

    UPROPERTY() bool  bZooming = false;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomedFOV     = 65.f;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomInterpSpeed= 20.f;
    UPROPERTY(EditAnywhere, Category="Combat") float DefaultFOV    = 90.f;
    float CurrentFOV = 90.f;

    // State
    bool bFireButtonPressed = false;
    bool bIsReloading = false;


    // Timers
    FTimerHandle ReloadTimerHandle;
    FTimerHandle FireTimerHandle;

     // Helpers to reduce duplication
    void ApplyPrimaryEquipSetup(AWeapon* WeaponToEquip); // keeps equip replication consistent
    bool ShouldSkipLocalMulticast() const; // avoids double-playing FX on owning client
    void PerformLocalAndServerFire(const FVector_NetQuantize& Target,
    void (UCombatComponent::*LocalFunc)(const FVector_NetQuantize&), void (UCombatComponent::*ServerFunc)(const FVector_NetQuantize&));
};

