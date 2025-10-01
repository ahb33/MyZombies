// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatState.h"          // UENUM used in UPROPERTY
#include "MyHUD.h"                // needed because FHUDPackage is a by-value member
#include "Weapon.h"
#include "CombatComponent.generated.h"

// Forward decls
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
    AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
    AWeapon* GetSecondaryWeapon() const {return SecondaryWeapon;}
    AMainCharacter* GetMainCharacter() const {return MainCharacter;}
    void SetAiming(bool bIsAiming);
    bool ShouldSwapWeapons() const;
    void SetCombatState(ECombatState State);
    ECombatState GetCombatState() const { return CombatState; }
    void TraceUnderCrosshairs(FHitResult& HitResult) const;
    void TraceFromCamera(FHitResult& HitResult) const;
    void TraceFromMuzzle(FHitResult& HitResult) const;
    bool IsCameraObstructed() const;
    void HandleZoom(float DeltaTime);
    void SetZooming(bool bIsZooming);
    bool IsZooming() const { return bZooming; }
    FVector_NetQuantize GetHitTarget() const;

    UFUNCTION() void OnRep_CombatState();
    void SetHUDCrosshairs(float DeltaTime);

    UFUNCTION(Server, Reliable) void ServerReload();
    UFUNCTION(Server, Reliable) void ServerSetAiming(bool bIsAiming);
    UFUNCTION(Server, Reliable) void ServerHitScanFire(const FVector_NetQuantize& TraceHitTarget);
    UFUNCTION(Server, Reliable) void ServerShotgunFire(const FVector_NetQuantize& TraceHitTarget);
    UFUNCTION() void OnRep_Aiming();
    UFUNCTION() void OnRep_EquippedWeapon();
    UFUNCTION() void OnRep_SecondaryWeapon();     


protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
    AWeapon* EquippedWeapon = nullptr;

    UPROPERTY(ReplicatedUsing=OnRep_SecondaryWeapon)
    AWeapon* SecondaryWeapon = nullptr;

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
    UPROPERTY(ReplicatedUsing=OnRep_Aiming) bool bAiming = false;
    UPROPERTY() bool  bZooming = false;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomedFOV     = 65.f;
    UPROPERTY(EditAnywhere, Category="Combat") float ZoomInterpSpeed= 20.f;
    UPROPERTY(EditAnywhere, Category="Combat") float DefaultFOV    = 90.f;
    float CurrentFOV = 90.f;

    // State
    UPROPERTY(ReplicatedUsing=OnRep_CombatState) ECombatState CombatState = ECombatState::ECS_Unoccupied;
    bool bCanFire = true;
    bool bFireButtonPressed = false;
    bool bIsReloading = false;

    // Timers
    FTimerHandle ReloadTimerHandle;
    FTimerHandle FireTimerHandle;
};

