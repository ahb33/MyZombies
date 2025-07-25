#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TurnInPlace.h"
#include "CombatState.h"
#include "MainCharacter.generated.h"

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
    class AWeapon* GetEquippedWeapon();

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
    void ServerEquipButtonPressed();

    // Utility Functions
    void SetOverlappingWeapon(class AWeapon* Weapon);
    void SetOverlappingItem(class APickUp* PickUp);

    // Animation Playback
    void PlayFireMontage(bool bAiming);
    void PlayReloadMontage();



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
    // Components
    UPROPERTY(VisibleAnywhere, Category = Camera)
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = Camera)
    class UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UCombatComponent* combatComponent;

    // Replication Variables
    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
    class AWeapon* OverlappingWeapon;

    UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
    float PlayerHealth;

    // Other Variables
    UPROPERTY(EditAnywhere, Category = Combat)
    class UAnimMontage* FireMontage;

    UPROPERTY(EditAnywhere, Category = Combat)
    class UAnimMontage* ReloadMontage;

    float MaxHealth;
    FRotator StartingAimRotation;
    float AO_Yaw;
    float AO_Pitch;
    float InterpAO_Yaw;
    ETurningInPlace TurningInPlace;

    class APickUp* OverlappingItem;
    class AHealthPickUp* pickUpHealth;

    class AMyPlayerController* MyPlayerController;
    class AMyHUD* MyGameHUD;

    // Replication Functions
    UFUNCTION()
    void OnRep_OverlappingWeapon();
    UFUNCTION()
    void OnRep_Health();


};