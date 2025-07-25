// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MainCharacter.h"
#include "MyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"   
#include "DrawDebugHelpers.h"           
#include "Engine/World.h"      
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/EngineTypes.h"         
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CharacterMovementComponent.h" 

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
    MainCharacter = Cast<AMainCharacter>(GetOwner());
	if (MainCharacter)
	{
		if (MainCharacter->GetFollowCamera())
		{
			DefaultFOV = MainCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
    
	// ...
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetEquippedWeapon())
    {
        HandleZoom(DeltaTime);
        SetHUDCrosshairs(DeltaTime);

        FHitResult HitResult;

        // If the camera is obstructed, trace from the weapon muzzle instead
        if (IsCameraObstructed())
        {
            TraceFromMuzzle(HitResult);
        }
        else
        {
            TraceFromCamera(HitResult);
        }

        // Update target to hit location or default to max range
        LocalHitTarget = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
    }
	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, CombatState); /*Combat State registered for replication
	for all clients no need for condition*/
	DOREPLIFETIME(UCombatComponent, bAiming);    
    DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
    DOREPLIFETIME(UCombatComponent, EquippedWeapon);

}


void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
    if (EquippedWeapon && !EquippedWeapon->WeaponIsEmpty())
    {
        bCanFire = false;
        EquippedWeapon->Fire(LocalHitTarget);
    }
}
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (!MainCharacter || !WeaponToEquip)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipWeapon failed: Invalid MainCharacter or WeaponToEquip."));
        return;
    }

    if (CombatState != ECombatState::ECS_Unoccupied) return;

    // if no weapon is currently equipped set WeaponToEquip to value of EquippedWeapon
    // #1 address the nulling of overlappingWeapon and #2 refactor checking for equippedweapon and so on

    // if EquippedWeapon is valid but SecondaryWeapon is not equip Secondary weapon
    if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
    {

        SecondaryWeapon = WeaponToEquip;
        AttachWeaponToWeaponSocket(SecondaryWeapon);
        SecondaryWeapon->SetOwner(MainCharacter);
        SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

    }
    else
    {        
        EquippedWeapon = WeaponToEquip;
        AttachActorToRightHand(EquippedWeapon);
        EquippedWeapon->SetOwner(MainCharacter);
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    }
}



void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
    if (!WeaponToEquip || !MainCharacter) return;

    SecondaryWeapon = WeaponToEquip;

    AttachWeaponToWeaponSocket(WeaponToEquip);

    // Hide the weapon's pickup widget
    WeaponToEquip->ShowPickUpWidget(false);
    WeaponToEquip->SetOwner(MainCharacter);

    UE_LOG(LogTemp, Log, TEXT("Secondary weapon equipped: %s"), *WeaponToEquip->GetName())

}

void UCombatComponent::SwapWeapons()
{
    if (!EquippedWeapon || !SecondaryWeapon) return;

    // Swap primary and secondary
    AWeapon* Temp = EquippedWeapon;
    EquippedWeapon = SecondaryWeapon;
    SecondaryWeapon = Temp;

    // Attach primary weapon to the right hand
    AttachActorToRightHand(EquippedWeapon);

    // Attach secondary weapon to the weapon socket
    AttachWeaponToWeaponSocket(SecondaryWeapon);
    EquippedWeapon->SetOwner(MainCharacter);

}

bool UCombatComponent::ShouldSwapWeapons() const
{
    if(EquippedWeapon && SecondaryWeapon) return true;

    return false;
}


// 2 issues: #1 weapon not attaching #2
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
    if (!MainCharacter || !ActorToAttach) return;

    const USkeletalMeshSocket* HandSocket = MainCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, MainCharacter->GetMesh());
    }
}

void UCombatComponent::AttachWeaponToWeaponSocket(AActor* WeaponToAttach)
{
	// check if character or weapon to attach are nullptr
	// create and define const ref to USkeletalMeshSocket 
	// check if ref is valid
	// AttachActor 
	if(MainCharacter == nullptr || MainCharacter->GetMesh() == nullptr || WeaponToAttach == nullptr) return;
	const USkeletalMeshSocket* WeaponSocket  = MainCharacter->GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket"));
	if(WeaponSocket)
	{
		WeaponSocket ->AttachActor(WeaponToAttach, MainCharacter->GetMesh());
	}
}

void UCombatComponent::Reload()
{

    if (GetEquippedWeapon())
    {
        UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Reload() called"));

        // Initiate reload on server
        ServerReload();

        // Play reload montage locally
        if (MainCharacter->IsLocallyControlled())
        {
            MainCharacter->PlayReloadMontage();
        }
        bIsReloading = true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipped Weapon is not valid"));
    }
}

void UCombatComponent::ServerReload_Implementation()
{
    if (!MainCharacter || !GetEquippedWeapon()) 
    {
        UE_LOG(LogTemp, Error, TEXT("ServerReload: Null MainCharacter or Weapon!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::ServerReload_Implementation() called"));

    // // Transition to reloading state
    SetCombatState(ECombatState::ECS_Reloading);

    // Handle ammo reloading
    int32 AmmoReload = AmmoToReload();
    GetEquippedWeapon()->ReloadAmmo(AmmoReload);
    GetEquippedWeapon()->RefreshHUD();



    float ReloadDuration = MainCharacter->GetReloadDuration();
    GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UCombatComponent::FinishReloading, ReloadDuration, false);
}

void UCombatComponent::FinishReloading()
{
    if (!GetEquippedWeapon()) return;

    UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::FinishReloading() called"));

    // Reset combat state
    SetCombatState(ECombatState::ECS_Unoccupied);

}

int32 UCombatComponent::AmmoToReload()
{

    UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::AmmoToReload() called"));

    int32 CurrentAmmoOnHand = GetEquippedWeapon()->GetCurrentAmmoOnHand();
    int32 CurrentAmmoInMag = GetEquippedWeapon()->GetCurrentAmmoInMag();
    int32 MaxAmmoOnHand = GetEquippedWeapon()->GetMaxAmmoOnHand();
    int32 MaxReloadAmount = MaxAmmoOnHand - CurrentAmmoOnHand;
    int32 AmmoToReload = FMath::Min(MaxReloadAmount, CurrentAmmoInMag);

    return AmmoToReload;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
    if(MainCharacter == nullptr || EquippedWeapon == nullptr) return;

    // The server will set the value here or in ServerSetAiming_Implementation
    bAiming = bIsAiming;
    ServerSetAiming(bAiming);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UCombatComponent::OnRep_Aiming()
{
	if (MainCharacter && MainCharacter->IsLocallyControlled())
	{
		bAiming = true;
	}
}

void UCombatComponent::SetCombatState(ECombatState State)
{
    CombatState = State;
    UE_LOG(LogTemp, Warning, TEXT("Combat State being called. Current state is %d"), static_cast<int32>(CombatState));    OnRep_CombatState();
    OnRep_CombatState();
}

void UCombatComponent::OnRep_CombatState()
{
    if (!MainCharacter) return;

    switch (CombatState)
    {
        case ECombatState::ECS_Unoccupied:
            if (bFireButtonPressed)
            {
                Fire();
            }
            break;

        case ECombatState::ECS_Reloading:
            if (MainCharacter && !MainCharacter->IsLocallyControlled()) MainCharacter->PlayReloadMontage();
            break;

        case ECombatState::ECS_Equipping:
            break;

        default:
            break;
    }
}

void UCombatComponent::SetZooming(bool bIsZooming)
{
    bZooming = bIsZooming;
}


void UCombatComponent::HandleZoom(float DeltaTime)
{
    if (GetEquippedWeapon() == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (MainCharacter && MainCharacter->GetFollowCamera())
	{

		MainCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
    if (!MainCharacter) return;

    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(MainCharacter->GetController());
    if (!PlayerController) return;

    HUD = HUD ? HUD : Cast<AMyHUD>(PlayerController->GetHUD());
    if (!HUD) return;

    HUDPackage.CrosshairsCenter = this->CrosshairsCenter;
    HUDPackage.CrosshairsLeft = this->CrosshairsLeft;
    HUDPackage.CrosshairsRight = this->CrosshairsRight;
    HUDPackage.CrosshairsTop = this->CrosshairsTop;
    HUDPackage.CrosshairsBottom = this->CrosshairsBottom;

    FVector2D SpeedRange(0.f, MainCharacter->GetCharacterMovement()->MaxWalkSpeed);
    FVector2D VelocityRange(0.f, 1.f);
    FVector Velocity = MainCharacter->GetVelocity();
    Velocity.Z = 0.f;

    CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(SpeedRange, VelocityRange, Velocity.Size());
    HUDPackage.CrosshairSpread = 0.0f + CrosshairVelocityFactor; // Adjusted base value

    HUD->SetHUDPackage(HUDPackage);
    
}

void UCombatComponent::TraceFromCamera(FHitResult& HitResult)
{
    FVector CameraLocation;
    FRotator CameraRotation;

    if (MainCharacter && MainCharacter->GetController())
    {
        MainCharacter->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
    }

    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraRotation.Vector() * TRACE_LENGTH);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(MainCharacter);
    if (GetEquippedWeapon()) Params.AddIgnoredActor(GetEquippedWeapon()); // Ignore weapon too

    GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);
}



void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
    if (!MainCharacter) return;

    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);

    // Set crosshair at the center of the screen
    FVector2D CrosshairPosition(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);

    FVector CrosshairWorldPosition, CrosshairWorldDirection;

    APlayerController* PlayerController = Cast<APlayerController>(MainCharacter->GetController());
    if (PlayerController && UGameplayStatics::DeprojectScreenToWorld(
            PlayerController, CrosshairPosition, CrosshairWorldPosition, CrosshairWorldDirection))
    {
        FVector TraceEnd = CrosshairWorldPosition + (CrosshairWorldDirection * TRACE_LENGTH);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(MainCharacter);
        if (GetEquippedWeapon()) Params.AddIgnoredActor(GetEquippedWeapon());

        GetWorld()->LineTraceSingleByChannel(HitResult, CrosshairWorldPosition, TraceEnd, ECC_Visibility, Params);
    }
// #if WITH_EDITOR
//     FColor DebugColor = bHit ? FColor::Red : FColor::Green;
//     DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, 1.f);
//     if (bHit)
//     {
//         DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Red, false, 1.f);
//     }
// #endif
}

void UCombatComponent::TraceFromMuzzle(FHitResult& HitResult)
{
    if (!EquippedWeapon || !MainCharacter) return;

    FVector MuzzleLocation = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(FName("Muzzle"));
    FVector ForwardVector = MainCharacter->GetControlRotation().Vector();
    FVector TraceEnd = MuzzleLocation + (ForwardVector * TRACE_LENGTH);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(MainCharacter);
    if (GetEquippedWeapon()) Params.AddIgnoredActor(GetEquippedWeapon());

    GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, TraceEnd, ECC_Visibility, Params);
}


bool UCombatComponent::IsCameraObstructed() const
{
    if (!MainCharacter || !MainCharacter->GetFollowCamera()) return false;

    FVector CameraLocation = MainCharacter->GetFollowCamera()->GetComponentLocation();
    FVector CharacterLocation = MainCharacter->GetActorLocation();
    
    FHitResult ObstructionHit;
    GetWorld()->LineTraceSingleByChannel(
        ObstructionHit, CameraLocation, CharacterLocation, ECC_Visibility
    );

    return ObstructionHit.bBlockingHit;
}




