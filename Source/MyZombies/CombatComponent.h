// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyHUD.h"
#include "CombatState.h"       
#include "CombatComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYZOMBIES_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	friend class AMainCharacter;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Firing Weapon   */
	void FireButtonPressed(bool bPressed);
	void Fire();

	/** Weapon Equipping */
	void EquipWeapon(class AWeapon*);
	void EquipSecondaryWeapon(class AWeapon*);
	void SwapWeapons();

    
    /** Weapon Reloading */
    void Reload();
    void FinishReloading();
    int32 AmmoToReload();
	virtual void ReloadAmmo(int32 Ammo) {};

	/** Utilities */
    void AttachActorToRightHand(AActor* ActorToAttach);
    void AttachWeaponToWeaponSocket(AActor* WeaponToAttach);
	class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
    void SetAiming(bool bIsAiming);
    bool ShouldSwapWeapons() const;
    void SetCombatState(ECombatState State);
    ECombatState GetCombatState() const { return CombatState; }
    void TraceUnderCrosshairs(FHitResult& HitResult);
    void TraceFromCamera(FHitResult& HitResult);
    void TraceFromMuzzle(FHitResult& HitResult);
    bool IsCameraObstructed() const;
    void HandleZoom(float DeltaTime); // will be called in tick

    void SetZooming(bool bIsZooming);
    bool IsZooming() const { return bZooming; }

    UFUNCTION()
    void OnRep_CombatState();
    void SetHUDCrosshairs(float DeltaTime);

    /** Server Functions */
    UFUNCTION(Server, Reliable)
    void ServerReload();



protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bIsAiming);

    // Replication
    UFUNCTION()
    void OnRep_Aiming();

private:	

    // Weapon References
    UPROPERTY(Replicated)
    class AWeapon* EquippedWeapon;

    UPROPERTY(Replicated)
    class AWeapon* SecondaryWeapon;

    // Character and Controller References
    class AMainCharacter* MainCharacter;
    class AMyPlayerController* Controller;

    UPROPERTY()
    AMyHUD* HUD;

    // Combat HUD
    FHUDPackage HUDPackage;
    FVector HitTarget;
    float CrosshairVelocityFactor;

    // Crosshair Details
    UPROPERTY(EditAnywhere, Category = Crosshairs)
    UTexture2D* CrosshairsCenter;

    UPROPERTY(EditAnywhere, Category = Crosshairs)
    UTexture2D* CrosshairsLeft;

    UPROPERTY(EditAnywhere, Category = Crosshairs)
    UTexture2D* CrosshairsRight;

    UPROPERTY(EditAnywhere, Category = Crosshairs)
    UTexture2D* CrosshairsTop;

    UPROPERTY(EditAnywhere, Category = Crosshairs)
    UTexture2D* CrosshairsBottom;

    UPROPERTY(ReplicatedUsing = OnRep_Aiming)
    bool bAiming;

    UPROPERTY()
    bool bZooming;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ZoomedFOV = 65.0f; // will change based on weapon

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ZoomInterpSpeed = 20.0f; // Speed for FOV interpolation

    UPROPERTY(EditAnywhere, Category = "Combat")
    float DefaultFOV = 90.0f; // FOV when not aiming; set in BeginPlay

    float CurrentFOV;

    UPROPERTY(ReplicatedUsing = OnRep_CombatState)
    ECombatState CombatState;

    /** Other Utility Variables */
    bool bCanFire; // Indicates if the weapon is reloading.
    bool bFireButtonPressed;
    bool bIsReloading;

    FVector LocalHitTarget;

    /** Timer Handles */
    FTimerHandle ReloadTimerHandle;


};
