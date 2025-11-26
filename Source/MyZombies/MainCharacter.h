#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TurnInPlace.h"      
#include "CombatState.h" 
#include "WeaponTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
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
class MYZOMBIES_API AMainCharacter : public ACharacter, public IGameplayTagAssetInterface
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

	// --- Gameplay tags ---
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& OutTags) const override
	{
		OutTags.AppendTags(CharacterTags);
	}

	// --- State / getters ---
	// Anim & view
	FORCEINLINE float GetCharacterYaw() const { return AO_Yaw; }
	FORCEINLINE float GetCharacterPitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Combat / controller
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	float GetReloadDuration() const;
	FVector GetHitTarget() const;
	ECombatState GetCharacterCombatState() const;
	AWeapon* GetEquippedWeapon() const;
	AMyPlayerController* GetMyPlayerController() const { return MyPlayerController; }

	// --- Health ---
	UFUNCTION(BlueprintPure, Category="Health Stats")
	float GetPlayerHealth() const { return PlayerHealth; }
	
	void SetPlayerHealth(float NewHealth);
	
	// Health setters that enforce authority and replication
	UFUNCTION(Server, Reliable) void Server_SetPlayerHealth(float NewValue);
	
	UFUNCTION(BlueprintPure, Category="Health Stats")
	float GetMaxHealth() const { return MaxHealth; }

	// Health authority API
	UFUNCTION()
	void UpdateHUDHealth() const;

	// Rep notifies (health)
	UFUNCTION()
	void OnRep_Health();

	// --- Inventory / pickups & overlap ---
	void SetOverlappingWeapon(AWeapon* Weapon);
	void SetOverlappingItem(APickUp* PickUp);

	// Rep notifies (overlap)
	UFUNCTION()
	void OnRep_OverlappingWeapon();

	// Ammo hook (wire to CombatComponent/Weapon later)
	UFUNCTION(BlueprintNativeEvent, Category="Combat")
	bool AddAmmoFromPickup(EWeaponType WeaponType, int32 Amount);
	virtual bool AddAmmoFromPickup_Implementation(EWeaponType WeaponType, int32 Amount);

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
	UFUNCTION(Server, Reliable) void Server_TryPickup(APickUp* Pickup);
	UFUNCTION(Server, Reliable) void ServerDie();

	// --- Anim montages ---
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

    void ApplyHealth_ServerOnly(float NewHealth); 

	
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

	UPROPERTY(ReplicatedUsing=OnRep_Health, EditAnywhere, Category="Stats")
	float PlayerHealth = 50.f;

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


};