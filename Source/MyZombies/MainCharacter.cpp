// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "Weapon.h"
#include "MyHUD.h"
#include "HealthPickUp.h"
#include "combatComponent.h"
#include "AmmoPickUp.h"
#include "MyPlayerController.h"
#include "Animation/AnimInstance.h" 
#include "WeaponTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"

// redo initializer list
AMainCharacter::AMainCharacter()
    : CameraBoom(CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"))),
      FollowCamera(CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"))),
      OverlappingWeapon(nullptr),
      PlayerHealth(100.f),
      FireMontage(nullptr),
      ReloadMontage(nullptr),
      MaxHealth(100.f),
      AO_Yaw(0.f),
      AO_Pitch(0.f),
      InterpAO_Yaw(0.f),
      TurningInPlace(ETurningInPlace::ETIP_NotTurning),
      OverlappingItem(nullptr),
      pickUpHealth(nullptr),
      MyPlayerController(nullptr),
      MyGameHUD(nullptr)
{
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

    combatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("ComponentComponent"));

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();

    MyPlayerController = Cast<AMyPlayerController>(GetController());
    if (MyPlayerController)
    {
        MyPlayerController->SetHUDHealth(PlayerHealth, MaxHealth);

        MyGameHUD = Cast<AMyHUD>(MyPlayerController->GetHUD());
        if (MyGameHUD)
        {
            MyGameHUD->AddCharacterStats();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to initialize MyPlayerController"));
    }

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
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{				
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
    InitValues();
				 
}

// hud 
void AMainCharacter::InitValues()
{
    if (!MyPlayerController)
    {
        MyPlayerController = Cast<AMyPlayerController>(Controller);
        if (MyPlayerController)
        {
            MyGameHUD = Cast<AMyHUD>(MyPlayerController->GetHUD());
            if (MyGameHUD)
            {
                UE_LOG(LogTemp, Warning, TEXT("MyGameHUD is valid"));
                MyGameHUD->AddCharacterStats();
            }
        }
    }

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
    if(combatComponent)
    {
        if(OverlappingWeapon)
        {
            if(HasAuthority())
            {
                ServerEquipButtonPressed();
            }
        }
        else if(!OverlappingWeapon && combatComponent->ShouldSwapWeapons())// else if no overlapping weapon is valid but equipp
        {
            // check where combat component is even being set
            combatComponent->SwapWeapons();
        }
    }
}

void AMainCharacter::ServerEquipButtonPressed_Implementation()
{
    // Directly call the equip logic for the server
    if (combatComponent)
    {
        if(OverlappingWeapon)
        {
            UE_LOG(LogTemp, Warning, TEXT("Overlapping weapon is valid"));
            combatComponent->EquipWeapon(OverlappingWeapon);

        }

        else if(combatComponent->ShouldSwapWeapons())
        {
            combatComponent->SwapWeapons();
        }

        UE_LOG(LogTemp, Warning, TEXT("ServerEquip function called"));
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
	else
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
}

bool AMainCharacter::IsWeaponEquipped() const
{
    return combatComponent && combatComponent->GetEquippedWeapon() != nullptr;
}

AWeapon* AMainCharacter::GetEquippedWeapon()
{
    return combatComponent ? combatComponent->GetEquippedWeapon() : nullptr;
}

void AMainCharacter::SetOverlappingItem(APickUp* PickUp)
{
	if (OverlappingItem)
	{
		OverlappingItem->ShowPickUpWidget(false);
	}

	OverlappingItem = PickUp;

	if(IsLocallyControlled())
	{
		if (PickUp)
		{
			PickUp->ShowPickUpWidget(true);

			pickUpHealth = Cast<AHealthPickUp>(PickUp); // Assign the value to the member variable
		}
	}

}



void AMainCharacter::PickUpButtonPressed()
{
	if (pickUpHealth && IsValid(pickUpHealth))
	{
		UE_LOG(LogTemp, Warning, TEXT("Calling AddHealth on pickUpHealth"));
		pickUpHealth->AddHealth(PlayerHealth, MaxHealth);
		pickUpHealth = nullptr; // Set the pickup reference to null
	}
	else

	{
		UE_LOG(LogTemp, Warning, TEXT("No overlapping health pickup"));
	}

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

bool AMainCharacter::IsReloading() const
{

    return false;

}


void AMainCharacter::PlayFireMontage(bool bAiming)
{

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && FireMontage)
    {
        FName SectionName = bAiming ? FName("FireIronSight") : FName("RifleHip");
        AnimInstance->Montage_JumpToSection(SectionName);
    }
}

void AMainCharacter::PlayReloadMontage()
{
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{	
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance And ReloadMontage correct") );
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

        EWeaponType WeaponType = GetEquippedWeapon()->GetWeaponType();
        switch (WeaponType)
        {
            case EWeaponType::EWT_AssaultRifle:
                UE_LOG(LogTemp, Warning, TEXT("Assault Rifle enum called"));
                SectionName = FName("Rifle");
                break;
            case EWeaponType::EWT_Shotgun:
                SectionName = FName("Shotgun");
                break;
            // Add cases for other weapon types
            default:
                UE_LOG(LogTemp, Error, TEXT("Unknown Weapon Type: %d"), static_cast<int32>(WeaponType));
                return; // Early exit to avoid calling with an invalid SectionName
        }

        AnimInstance->Montage_JumpToSection(SectionName);
	}
}


float AMainCharacter::GetReloadDuration() const
{
    if (this->ReloadMontage)
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


    FVector velocity = GetVelocity();
    FVector lateralSpeed = FVector(velocity.X, velocity.Y, 0.f);
    float movementSpeed = lateralSpeed.Size();

    bool bIsInAir = GetCharacterMovement()->IsFalling();

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

void AMainCharacter::OnRep_Health()
{

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

