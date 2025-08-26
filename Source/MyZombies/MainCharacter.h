#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TurnInPlace.h"      
#include "CombatState.h"      
#include "MainCharacter.generated.h"

// forward-declares:
class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class AWeapon;
class APickUp;
class AHealthPickUp;
class AMyPlayerController;
class AMyHUD;
class UAnimMontage;
class UInputComponent;
class UMyGameInstance;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainCharacterDeath);

UCLASS()
class MYZOMBIES_API AMainCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMainCharacter();

    virtual void PostInitializeComponents() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser) override; //abstr to base?
    void Die(); // abstr to base?

    // Initialization
    void InitValues();

    // Private Input Handlers
    bool IsWeaponEquipped() const;
    bool IsAiming() const;
    bool IsReloading() const;


    // Getters
    FORCEINLINE float GetCharacterYaw() const { return AO_Yaw; }
    FORCEINLINE float GetCharacterPitch() const { return AO_Pitch; }
    FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    float GetReloadDuration() const;
    FVector GetHitTarget() const;
    class AMyPlayerController* GetMyPlayerController() const { return MyPlayerController; }
    ECombatState GetCharacterCombatState() const;
    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetPlayerHealth() const { return PlayerHealth; }
    class AWeapon* GetEquippedWeapon() const;

    // Input Actions
    void EquipButtonPressed();
    void CrouchButtonPressed();
    void AimButtonPressed();
    void AimButtonReleased();
    void FireButtonPressed();
    void FireButtonReleased();
    void ReloadButtonPressed();
    void PickUpButtonPressed();
    void ZoomButtonPressed();
    void ZoomButtonReleased();

    // Replication Functions
    UFUNCTION(Server, Reliable)
    void Server_EquipButtonPressed();
    // von onRepHealth

    // Utility Functions
    void SetOverlappingWeapon(class AWeapon* Weapon);
    void SetOverlappingItem(class APickUp* PickUp);

    // Animation Playback
    void PlayFireMontage(bool bAiming);
    void PlayReloadMontage();

    FOnMainCharacterDeath OnMainCharacterDeath;

protected:
    virtual void BeginPlay() override;

    // Movement Functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);

    // Combat Functions
    void AimOffset(float DeltaTime);
    void TurnInPlace(float DeltaTime);



private:

    // -- UE-reflected components & assets --
    UPROPERTY(VisibleAnywhere, Category="Camera")
    USpringArmComponent* CameraBoom = nullptr;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* FollowCamera = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
    UCombatComponent* combatComponent = nullptr;

    UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
    AWeapon* OverlappingWeapon = nullptr;

    UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Stats")
    float PlayerHealth = 100.f;

    UPROPERTY(EditAnywhere, Category="Combat")
    UAnimMontage* FireMontage = nullptr;

    UPROPERTY(EditAnywhere, Category="Combat")
    UAnimMontage* ReloadMontage = nullptr;

    // -- pure runtime refs (no UPROPERTY needed) --
    APickUp* OverlappingItem = nullptr;
    AHealthPickUp* pickUpHealth = nullptr;
    AMyPlayerController* MyPlayerController = nullptr;
    AMyHUD* MyGameHUD = nullptr;
    UMyGameInstance* MyGameInstanceRef = nullptr;

    // -- simple PODs with in-class defaults --
    float MaxHealth = 100.f;
    FRotator StartingAimRotation;
    float AO_Yaw = 0.f, AO_Pitch = 0.f, InterpAO_Yaw = 0.f;
    ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;
    FTimerHandle DestructionTimer; // Timer handle for delayed destruction


    // -- rep notifies --
    UFUNCTION() void OnRep_OverlappingWeapon();
    UFUNCTION() void OnRep_Health();


};