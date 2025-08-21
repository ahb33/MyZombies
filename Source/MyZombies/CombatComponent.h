// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatState.h"          // UENUM used in UPROPERTY
#include "MyHUD.h"                // needed because FHUDPackage is a by-value member
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

    void EquipWeapon(AWeapon*);
    void EquipSecondaryWeapon(AWeapon*);
    void SwapWeapons();

    void Reload();
    void FinishReloading();
    int32 AmmoToReload();
    virtual void ReloadAmmo(int32 Ammo) {}

    void AttachActorToRightHand(AActor* ActorToAttach);
    void AttachWeaponToWeaponSocket(AActor* WeaponToAttach);
    AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
    void SetAiming(bool bIsAiming);
    bool ShouldSwapWeapons() const;
    void SetCombatState(ECombatState State);
    ECombatState GetCombatState() const { return CombatState; }
    void TraceUnderCrosshairs(FHitResult& HitResult);
    void TraceFromCamera(FHitResult& HitResult);
    void TraceFromMuzzle(FHitResult& HitResult);
    bool IsCameraObstructed() const;
    void HandleZoom(float DeltaTime);
    void SetZooming(bool bIsZooming);
    bool IsZooming() const { return bZooming; }

    UFUNCTION() void OnRep_CombatState();
    void SetHUDCrosshairs(float DeltaTime);

    UFUNCTION(Server, Reliable) void ServerReload();
    UFUNCTION(Server, Reliable) void ServerSetAiming(bool bIsAiming);
    UFUNCTION()               void OnRep_Aiming();

protected:
    virtual void BeginPlay() override;

private:
    // Replicated refs
    UPROPERTY(Replicated) AWeapon* EquippedWeapon = nullptr;
    UPROPERTY(Replicated) AWeapon* SecondaryWeapon = nullptr;

    // Local refs
    AMainCharacter*      MainCharacter = nullptr;
    AMyPlayerController* Controller     = nullptr;

    UPROPERTY() AMyHUD* HUD = nullptr;

    // HUD data (requires MyHUD.h here because it's by value)
    FHUDPackage HUDPackage{};
    FVector     HitTarget = FVector::ZeroVector;
    float       CrosshairVelocityFactor = 0.f;

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

    FVector LocalHitTarget = FVector::ZeroVector;

    // Timers
    FTimerHandle ReloadTimerHandle;
};

