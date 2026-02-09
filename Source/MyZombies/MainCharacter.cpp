// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "CombatComponent.h"
#include "Weapon.h"
#include "HealthPickUp.h"
#include "Animation/AnimInstance.h"
#include "ZombiesGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "DamageHelpers.h"
#include "MyPlayerController.h"
#include "CCDebug.h"

// redo initializer list
AMainCharacter::AMainCharacter()
    : CameraBoom(CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"))),
    FollowCamera(CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"))),
    combatComponent(CreateDefaultSubobject<UCombatComponent>(TEXT("ComponentComponent")))
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;

    // Attach the camera boom to the mesh to allow it to follow the character's movements (e.g., crouching)
    CameraBoom->SetupAttachment(GetMesh());
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Attach the follow camera to the camera boom
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Character movement settings
    bUseControllerRotationYaw = false; // Let the character movement handle rotation
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // Enable crouching
    CharacterTags.AddTag(FGameplayTag::RequestGameplayTag("Faction.Player"));

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitValues();
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);


	PlayerInputComponent->BindAxis(TEXT("Move Forward"), this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Move Right"), this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn Right"), this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("Look Up"), this, &AMainCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMainCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMainCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMainCharacter::AimButtonReleased);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMainCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMainCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("Reload", IE_Released, this, &AMainCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &AMainCharacter::PickUpButtonPressed);
    PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AMainCharacter::ZoomButtonPressed);
    PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AMainCharacter::ZoomButtonReleased);

}

void AMainCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AMainCharacter, OverlappingWeapon, COND_OwnerOnly);

    DOREPLIFETIME(AMainCharacter, PlayerHealth);
    DOREPLIFETIME(AMainCharacter, CharacterTags);
    DOREPLIFETIME(AMainCharacter, bIsDead);
    DOREPLIFETIME(AMainCharacter, ReplicatedAimYaw);
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{				
	Super::Tick(DeltaTime);

    // Server computes aim-yaw for everyone (server receives clients' control rotation)
    if (HasAuthority() && Controller)
    {
        const FRotator Aim(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FRotator Actor(0.f, GetActorRotation().Yaw, 0.f);
        ReplicatedAimYaw = UKismetMathLibrary::NormalizedDeltaRotator(Aim, Actor).Yaw;
    }
	AimOffset(DeltaTime);				 
}

// hud 
void AMainCharacter::InitValues()
{
    if (bUIInitDone) return;   

    if (AMyPlayerController* PC = Cast<AMyPlayerController>(Controller))
    {
        if (AMyHUD* HUD = Cast<AMyHUD>(PC->GetHUD()))
        {
            HUD->AddCharacterStats();
            PC->SetHUDHealth(PlayerHealth, MaxHealth);

            MyPlayerController = PC;
            MyGameHUD = HUD;
            bUIInitDone = true;
        }
    }

}


void AMainCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);     // server
    InitValues();
}

void AMainCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    InitValues();
}

void AMainCharacter::UnPossessed()
{
    // Unbind delegates here if you bound any.
    Super::UnPossessed();
}

void AMainCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (combatComponent)
	{
		combatComponent->MainCharacter = this;
	}
}

// Movement Functions
void AMainCharacter::MoveForward(float value)
{
    if (Controller != nullptr && value != 0.f)
    {
        const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(direction, value);
    }
}

void AMainCharacter::MoveRight(float value)
{
    if (Controller != nullptr && value != 0.f)
    {
        const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(direction, value);
    }
}

void AMainCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void AMainCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}	

void AMainCharacter::EquipButtonPressed()
{
    if (!combatComponent)
    {
        return; // nothing to do without our combat bit
    }

    // If I’m a client, just ask the server to handle equip/swap
    if (!HasAuthority())
    {
        Server_EquipButtonPressed();
        return;
    }

    // On the server: either pick up the nearby weapon or swap out your current one
    if (OverlappingWeapon)
    {
        combatComponent->EquipWeapon(OverlappingWeapon);
    }
    else if (combatComponent->ShouldSwapWeapons())
    {
        combatComponent->SwapWeapons();
    }
}

void AMainCharacter::Server_EquipButtonPressed_Implementation()
{
    // Server-side copy of the same logic—
    // this is where the real equip/swap actually happens
    if (OverlappingWeapon)
    {
        combatComponent->EquipWeapon(OverlappingWeapon);
    }
    else if (combatComponent->ShouldSwapWeapons())
    {
        combatComponent->SwapWeapons();
    }
}

void AMainCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
    // Prevent overlapping with the currently equipped weapon
    if (OverlappingWeapon)
    {
        OverlappingWeapon->ShowPickUpWidget(false);
    }

    OverlappingWeapon = Weapon;

    if (IsLocallyControlled() && OverlappingWeapon)
    {
        OverlappingWeapon->ShowPickUpWidget(true);
    }
}

void AMainCharacter::OnRep_OverlappingWeapon()
{
    if (OverlappingWeapon)
    {
        OverlappingWeapon->ShowPickUpWidget(true);
    }
}

bool AMainCharacter::IsWeaponEquipped() const
{
    return combatComponent && combatComponent->GetEquippedWeapon() != nullptr;
}

AWeapon* AMainCharacter::GetEquippedWeapon() const
{
    return combatComponent ? combatComponent->GetEquippedWeapon() : nullptr;
}

AWeapon* AMainCharacter::GetSecondaryWeapon() const
{
    return combatComponent ? combatComponent->GetSecondaryWeapon() : nullptr;
}

void AMainCharacter::SetOverlappingItem(APickUp* PickUp)
{
	if (OverlappingItem)
	{
		OverlappingItem->ShowPickUpWidget(false);
	}
    
	OverlappingItem = PickUp;
    
	if (IsLocallyControlled() && OverlappingItem)
	{
		OverlappingItem->ShowPickUpWidget(true);
	}
}

bool AMainCharacter::AddAmmoFromPickup_Implementation(EWeaponType InType, int32 Amount)
{
	if (InType == EWeaponType::None || Amount <= 0) return false;

	AWeapon* W = GetEquippedWeapon();
	if (!W) return false;  // no weapon -> don't consume pickup
	if (W->GetWeaponType() != InType) return false; // wrong ammo type -> don't consume

	const int32 Reserve = W->GetCurrentAmmoOnHand();
	const int32 MaxReserve = W->GetMaxAmmoOnHand();
	const int32 NewReserve = FMath::Clamp(Reserve + Amount, 0, MaxReserve);

	if (NewReserve == Reserve) return false;  // already full -> don't consume

	W->SetAmmo(NewReserve, W->GetCurrentAmmoInMag()); // (OnHand, InMag)
	W->ForceNetUpdate();  // helps push OwnerOnly ammo replication faster
	return true;
}

void AMainCharacter::PickUpButtonPressed()
{
    if (!OverlappingItem) 
    {
        return;
    }


    // why: authoritative consume; single path
    Server_TryPickup(OverlappingItem);
}

void AMainCharacter::Server_TryPickup_Implementation(APickUp* Pickup)
{
    if (!Pickup) return;

    
    if (!Pickup->IsOverlappingActor(this)) return;

    if (Pickup->TryConsume(this))
    {
        OverlappingItem = nullptr; // stop showing prompts after consume
    }
}


void AMainCharacter::SetPlayerHealth(float NewHealth)
{
    const float Clamped = FMath::Clamp(NewHealth, 0.f, MaxHealth);
    if (HasAuthority())
    {
        PlayerHealth = Clamped;    
        if (IsLocallyControlled()) UpdateHUDHealth();  
    }
    else
    {
        Server_SetPlayerHealth(Clamped);        // client requests server to set
    }
}


void AMainCharacter::Server_SetPlayerHealth_Implementation(float NewHealth) 
{
    PlayerHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);     // runs on server
    if (IsLocallyControlled()) UpdateHUDHealth();
}

void AMainCharacter::UpdateHUDHealth() const
{
    if (auto* PC = Cast<AMyPlayerController>(GetController()))
    PC->SetHUDHealth(PlayerHealth, MaxHealth);
}

void AMainCharacter::OnRep_Health()
{
	UpdateHUDHealth();
}

void AMainCharacter::ZoomButtonPressed()
{
    if (combatComponent)
    {
        combatComponent->SetZooming(true); // Trigger zooming
    }
}

void AMainCharacter::ZoomButtonReleased()
{
    if (combatComponent)
    {
        combatComponent->SetZooming(false); // Stop zooming
    }
}

void AMainCharacter::FireButtonPressed()
{
	if(combatComponent)
	{
		combatComponent->FireButtonPressed(true);
	}

}

void AMainCharacter::FireButtonReleased()
{
	if(combatComponent)
	{
		combatComponent->FireButtonPressed(false);
	}
}

void AMainCharacter::ReloadButtonPressed()
{
    if(combatComponent)
	{
		combatComponent->Reload();
	}
}

void AMainCharacter::AimButtonPressed()
{	
    if(combatComponent)
    {
        combatComponent->SetAiming(true);
    }

}


void AMainCharacter::AimButtonReleased()
{
    if(combatComponent)
    {
        combatComponent->SetAiming(false);
    }
}

bool AMainCharacter::IsAiming() const
{
    return (combatComponent && combatComponent->bAiming);
}


void AMainCharacter::PlayFireMontage(bool bAiming)
{

    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        if (FireMontage)
        {
            const FName SectionName = bAiming ? FName("FireIronSight") : FName("RifleHip");
            AnimInstance->Montage_JumpToSection(SectionName);
        }
    }
}

void AMainCharacter::PlayReloadMontage()
{
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        if (ReloadMontage)
        {
            AnimInstance->Montage_Play(ReloadMontage);
            FName SectionName;

            EWeaponType WeaponType = GetEquippedWeapon()->GetWeaponType();
            switch (WeaponType)
            {
                case EWeaponType::AssaultRifle: UE_LOG(LogTemp, Warning, TEXT("Assault Rifle enum called")); SectionName = FName("Rifle"); break;
                case EWeaponType::Shotgun:      SectionName = FName("Shotgun"); break;
                default: UE_LOG(LogTemp, Error, TEXT("Unknown Weapon Type: %d"), static_cast<int32>(WeaponType)); return;
            }
            AnimInstance->Montage_JumpToSection(SectionName);
        }
    }
}


float AMainCharacter::GetReloadDuration() const
{
    if (!ReloadMontage) return 0.0f; 
    {
        return this->ReloadMontage->GetPlayLength();
    }
    return 0.0f;
}


void AMainCharacter::CrouchButtonPressed()
{
	if(bIsCrouched) 
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

// Aim Offset Functions
void AMainCharacter::AimOffset(float DeltaTime)
{
    const FVector velocity = GetVelocity();
    FVector lateralSpeed = FVector(velocity.X, velocity.Y, 0.f);
    const float movementSpeed = lateralSpeed.Size();
    const bool bIsInAir = GetCharacterMovement()->IsFalling();

    if (movementSpeed == 0.f && !bIsInAir)
    {
        FRotator currentAimRotation(0.f, GetBaseAimRotation().Yaw, 0.f);
        FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, StartingAimRotation);
        AO_Yaw = deltaAimRotation.Yaw;
        bUseControllerRotationYaw = false;
        TurnInPlace(DeltaTime);
    }
    else
    {
        StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
        AO_Yaw = 0.f;
        bUseControllerRotationYaw = true;
        TurningInPlace = ETurningInPlace::ETIP_NotTurning;
    }
    AO_Pitch = GetBaseAimRotation().Pitch;
    if (!IsLocallyControlled() && AO_Pitch > 90.f)
    {
        AO_Pitch = FMath::GetMappedRangeValueClamped(FVector2D(270.f, 360.f), FVector2D(-90.f, 0.f), AO_Pitch);
    }

    if (!IsLocallyControlled())
    {
        AO_Yaw = ReplicatedAimYaw;
    }
}

void AMainCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}


ECombatState AMainCharacter::GetCharacterCombatState() const
{
	if (combatComponent == nullptr) return ECombatState::ECS_MAX;
	return combatComponent->GetCombatState();
}


FVector AMainCharacter::GetHitTarget() const
{
	// if(combatComponent == nullptr)
     return FVector();
	// return combatComponent->GetLocalHitTarget();
}

// TakeDamage(Server)
float AMainCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.f;

    CCDBG_SCOPE(this, "TakeDamage");

    const APawn* Killer = DamageHelpers::ResolveKillerPawn(EventInstigator, DamageCauser);

    if (DamageHelpers::IsZombiesMode(GetWorld()))
    {
        // Zombies mode: only zombies may hurt the player
        if (!DamageHelpers::IsZombieActor(Killer)) return 0.f;
    }

    const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // health update using your variables
    SetPlayerHealth(PlayerHealth - Applied);

    if (PlayerHealth <= 0.f && !bIsDead) Die();

    return Applied;
}



void AMainCharacter::Die()
{
    CCDBG_SCOPE(this, "Die");

    
    // Forward to server if called on a client.
	if (!HasAuthority())
	{
		ServerDie();
		return;
	}

    if (bIsDead) return;
    if (AZombiesGameMode* GM = GetWorld()->GetAuthGameMode<AZombiesGameMode>()) GM->HandlePlayerDeath(Controller, nullptr);

    CCDBG(this, TEXT("Die start  HasAuth=%d  bIsDead=%d"), HasAuthority()?1:0, bIsDead?1:0);
    bIsDead = true;  
    CCDBG(this, TEXT("Die start  HasAuth=%d  bIsDead=%d"), HasAuthority()?1:0, bIsDead?1:0);

    SetNetDormancy(DORM_Awake); // ensure awake for this burst

    if(GetEquippedWeapon()) GetEquippedWeapon()->SetWeaponState(EWeaponState::Dropped);
    if(GetSecondaryWeapon()) GetSecondaryWeapon()->SetWeaponState(EWeaponState::Dropped);

    SetLifeSpan(0.3f);
}

void AMainCharacter::ServerDie_Implementation()
{
	Die();
}



