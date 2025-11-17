#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TurnInPlace.h"      
#include "CombatState.h" 
#include "WeaponTypes.h"
#include "GameplayTagContainer.h"
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
class USoundBase;  


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainCharacterDeath);

UCLASS()
class MYZOMBIES_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// --- Construction & core overrides ---
	AMainCharacter();
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;
	void Die();

	// --- Initialization ---
	void InitValues();

	// --- Input state helpers ---
	bool IsWeaponEquipped() const;
	bool IsAiming() const;

	// --- Getters ---
	FORCEINLINE float GetCharacterYaw() const { return AO_Yaw; }
	FORCEINLINE float GetCharacterPitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	float GetReloadDuration() const;
	FVector GetHitTarget() const;
	AMyPlayerController* GetMyPlayerController() const { return MyPlayerController; }
	ECombatState GetCharacterCombatState() const;

	UFUNCTION(BlueprintPure, Category="Health Stats")
	float GetPlayerHealth() const { return PlayerHealth; }

	UFUNCTION(BlueprintPure, Category="Health Stats")
	float GetMaxHealth() const {return MaxHealth;}

	AWeapon* GetEquippedWeapon() const;

	// --- Input actions ---
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

	// --- RPCs ---
	UFUNCTION(Server, Reliable) void Server_EquipButtonPressed();
	UFUNCTION(Server, Reliable) void ServerDie();
	UFUNCTION(Server, Reliable) void Server_TryPickup(APickUp* Pickup);


	// Health authority API
	UFUNCTION(BlueprintCallable, Category="Stats")
	void SetHealth(float NewValue);

	UFUNCTION(Server, Reliable) 
	void Server_SetHealth(float NewValue);


	// Ammo hook (wire to CombatComponent/Weapon later)
	UFUNCTION(BlueprintNativeEvent, Category="Combat")
	bool AddAmmoFromPickup(EWeaponType WeaponType, int32 Amount);
	virtual bool AddAmmoFromPickup_Implementation(EWeaponType WeaponType, int32 Amount);



	// --- Utility ---
	void SetOverlappingWeapon(AWeapon* Weapon);
	void SetOverlappingItem(APickUp* PickUp);
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();

	// --- Events ---
	FOnMainCharacterDeath OnMainCharacterDeath;

protected:
	// --- Lifecycle ---
	virtual void BeginPlay() override;

	// --- Movement ---
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// --- Combat/aim ---
	void AimOffset(float DeltaTime);
	void TurnInPlace(float DeltaTime);

private:
	// --- Components & replicated refs ---
	UPROPERTY(VisibleAnywhere, Category="Camera")
	USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Camera")
	UCameraComponent* FollowCamera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UCombatComponent* combatComponent = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon = nullptr;

    UPROPERTY(Transient) bool bLastKillerWasZombie = false;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Stats")
	float PlayerHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Stats")
	float MaxHealth = 100.f;

	// --- Assets ---
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* FireMontage = nullptr;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ReloadMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Audio")
	USoundBase* DeathSFX = nullptr;

	 AHealthPickUp* pickUpHealth = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere, Category="Tags", Replicated, meta=(AllowPrivateAccess="true"))
	FGameplayTagContainer CharacterTags;

	// --- Runtime (non-UObject ownership) ---
	APickUp* OverlappingItem = nullptr;
	AMyPlayerController* MyPlayerController = nullptr;
	AMyHUD* MyGameHUD = nullptr;
	UMyGameInstance* MyGameInstanceRef = nullptr;

	// --- POD state ---
	FRotator StartingAimRotation;
	float AO_Yaw = 0.f, AO_Pitch = 0.f, InterpAO_Yaw = 0.f;
	ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	FTimerHandle DestructionTimer;

	// --- Rep notifies ---
	UFUNCTION() void OnRep_OverlappingWeapon();
	UFUNCTION() void OnRep_Health();
};