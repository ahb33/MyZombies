// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon.h"
#include "MainCharacter.h"
#include "MyPlayerController.h"
#include "Camera/CameraComponent.h"          
#include "Engine/SkeletalMeshSocket.h"   
#include "Kismet/GameplayStatics.h"
#include "Shotgun.h"      
#include "ProjectileWeapon.h"
#include "WeaponTypes.h"
#include "HitScanWeapon.h"
#include "Engine/EngineTypes.h"
#include "Templates/UnrealTemplate.h"
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
    if (!GetEquippedWeapon() || GetEquippedWeapon()->WeaponIsEmpty()) return;
    {
        // bCanFire = false;
        // add bool function CanFire()
        switch(GetEquippedWeapon()->GetWeaponType())
        {
            case EWeaponType::AssaultRifle:
            FireHitScanWeapon();
            break;

            case EWeaponType::Shotgun:
            FireShotgun();
            break;

            case EWeaponType::None:
            default:
            break;
        }
        // start fire cooldown per-weapon
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                FireTimerHandle, this, &UCombatComponent::OnFireCooldownFinished,
                GetEquippedWeapon()->GetFireDelay(), false);
        }
    }
}

void UCombatComponent::FireHitScanWeapon()
{
    if(!GetMainCharacter()) return;
    if(AHitScanWeapon* HitScanWeapon = Cast<AHitScanWeapon>(GetEquippedWeapon()))
    {
        const FVector_NetQuantize Target = GetHitTarget();
    
        if (!MainCharacter->HasAuthority())        // client cosmetic
        {
            HitScanLocalFire(Target);
        }   
        ServerHitScanFire(Target);                 // authoritative
    }
}

void UCombatComponent::FireShotgun()
{
    if(!GetMainCharacter()) return;
    if (AShotgun* Shotgun = Cast<AShotgun>(GetEquippedWeapon()))
    {
        const FVector_NetQuantize Target = GetHitTarget();

        if (!MainCharacter->HasAuthority())
        {
            ShotgunLocalFire(Target);
        }
        ServerShotgunFire(Target); 
    }
}


void UCombatComponent::HitScanLocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (GetMainCharacter() && CombatState == ECombatState::ECS_Unoccupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("Calling LocalFire"));

        if(AWeapon* W = GetEquippedWeapon())
        {
            W->Fire(TraceHitTarget);
            W->PlayFireEffects();
        }
    }
}

void UCombatComponent::ShotgunLocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (AShotgun* Shotgun = Cast<AShotgun>(GetEquippedWeapon()))
    {
        bIsReloading = false;
        MainCharacter->PlayFireMontage(bAiming);
        Shotgun->Fire(TraceHitTarget);         // scatter happens inside shotgun
        CombatState = ECombatState::ECS_Unoccupied;
    }
}

void UCombatComponent::ServerHitScanFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    UE_LOG(LogTemp, Warning, TEXT("ServerHitScanFire called: spawning authoritative HitScan"));
    MulticastFireHitScan_Implementation(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{   
    MulticastFireShotgun_Implementation(TraceHitTarget);
}

void UCombatComponent::MulticastFireHitScan_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    UE_LOG(LogTemp, Warning, TEXT("MulticastFireHitScan called on %s | LocallyControlled=%d | Authority=%d"), *GetName(),
    GetMainCharacter()->IsLocallyControlled(),
    GetMainCharacter()->HasAuthority());
    
    if (GetMainCharacter() && GetMainCharacter()->IsLocallyControlled() && !GetMainCharacter()->HasAuthority()) return;
    HitScanLocalFire(TraceHitTarget);
}

void UCombatComponent::MulticastFireShotgun_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (GetMainCharacter() && GetMainCharacter()->IsLocallyControlled() && !GetMainCharacter()->HasAuthority()) return;
    ShotgunLocalFire(TraceHitTarget);
}




void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (!MainCharacter || !WeaponToEquip) return;

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        // Client → ask server
        ServerEquipWeapon(WeaponToEquip);
        return;
    }

    // Server logic
    if (EquippedWeapon && !SecondaryWeapon)
    {
        EquipSecondaryWeapon(WeaponToEquip);
    }
    else
    {
        EquipPrimaryWeapon(WeaponToEquip);
    }
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
    if (!WeaponToEquip || !MainCharacter) return;

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    EquippedWeapon->SetOwner(MainCharacter);

    AttachActorToRightHand(EquippedWeapon);
    EquippedWeapon->ShowPickUpWidget(false);

    // Server host updates HUD instantly
    if (MainCharacter->IsLocallyControlled())
    {
        GetEquippedWeapon()->RefreshHUD();
    }

    // Adjust character rotation settings for aiming
    MainCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
    MainCharacter->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
    if (!WeaponToEquip || !MainCharacter) return;

    SecondaryWeapon = WeaponToEquip;
    SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
    SecondaryWeapon->SetOwner(MainCharacter);

    AttachWeaponToBackSocket(SecondaryWeapon);
    SecondaryWeapon->ShowPickUpWidget(false);
}
void UCombatComponent::OnRep_EquippedWeapon()
{
    if (GetEquippedWeapon() && GetMainCharacter())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnRep equipped weapon called"));
        AttachActorToRightHand(GetEquippedWeapon());
        GetEquippedWeapon()->ShowPickUpWidget(false);
        GetEquippedWeapon()->RefreshHUD();
    }
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
    if (GetSecondaryWeapon() && GetMainCharacter())
    {
        AttachWeaponToBackSocket(SecondaryWeapon);
        SecondaryWeapon->ShowPickUpWidget(false);
    }
}

void UCombatComponent::ServerEquipWeapon_Implementation(AWeapon* WeaponToEquip)
{
    if (!WeaponToEquip || !MainCharacter) return;

   EquipWeapon(WeaponToEquip);
}

void UCombatComponent::SwapWeapons()
{
    if (!EquippedWeapon || !SecondaryWeapon) return;

    // Swap primary and secondary
    Swap(EquippedWeapon, SecondaryWeapon);

    // Attach primary weapon to the right hand
    AttachActorToRightHand(EquippedWeapon);
    EquippedWeapon->SetOwner(MainCharacter);
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

    AttachWeaponToBackSocket(SecondaryWeapon);
    SecondaryWeapon->SetOwner(MainCharacter);
    SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);


    GetEquippedWeapon()->RefreshHUD();

}

bool UCombatComponent::ShouldSwapWeapons() const
{
    return EquippedWeapon && SecondaryWeapon;
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

void UCombatComponent::AttachWeaponToBackSocket(AActor* WeaponToAttach)
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

    if (GetEquippedWeapon() && !bIsReloading && GetCombatState() == ECombatState::ECS_Unoccupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Reload() called"));

        // Initiate reload on server
        ServerReload();
        if (GetMainCharacter()->IsLocallyControlled())
        {
            GetMainCharacter()->PlayReloadMontage();
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
    if (!GetMainCharacter() || !GetEquippedWeapon()) 
    {
        UE_LOG(LogTemp, Error, TEXT("ServerReload: Null MainCharacter or Weapon!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::ServerReload_Implementation() called"));

    // // Transition to reloading state
    SetCombatState(ECombatState::ECS_Reloading);
    int32 AmmoReload = AmmoToReload();
    GetEquippedWeapon()->ReloadAmmo(AmmoReload);
    GetEquippedWeapon()->RefreshHUD();



    float ReloadDuration = MainCharacter->GetReloadDuration();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ReloadTimerHandle, this, &UCombatComponent::FinishReloading, ReloadDuration, false);
    }

}


void UCombatComponent::OnFireCooldownFinished()
{
    bCanFire = true;
    if (bFireButtonPressed && CombatState == ECombatState::ECS_Unoccupied)
    {
        Fire(); // simple auto/semi handling
    }
}

void UCombatComponent::FinishReloading()
{
    if (!GetEquippedWeapon()) return;
    bIsReloading = false;

    UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::FinishReloading() called"));
    // Reset combat state if player has authority
    if(GetMainCharacter()->HasAuthority()) SetCombatState(ECombatState::ECS_Unoccupied);

}

int32 UCombatComponent::AmmoToReload() const
{
    UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::AmmoToReload() called"));

    const AWeapon* W = GetEquippedWeapon();
    if (!W) return 0;

    const int32 CurrentAmmoOnHand = W->GetCurrentAmmoOnHand();
    const int32 CurrentAmmoInMag  = W->GetCurrentAmmoInMag();
    const int32 MagCapacity       = W->GetMagCapacity();

    const int32 Need   = FMath::Max(0, MagCapacity - CurrentAmmoInMag);
    const int32 ToLoad = FMath::Min(Need, CurrentAmmoOnHand);
    return ToLoad;
}

void UCombatComponent::SetAiming(bool bIsAimingIn)
{
    if (!MainCharacter || !EquippedWeapon) return;
    if (GetOwnerRole() < ROLE_Authority)
    {
        ServerSetAiming(bIsAimingIn);
    }
    bAiming = bIsAimingIn;   // local responsiveness
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAimingIn)
{
    bAiming = bIsAimingIn;
}

void UCombatComponent::OnRep_Aiming()
{

}



void UCombatComponent::SetCombatState(ECombatState State)
{
    CombatState = State;
    UE_LOG(LogTemp, Warning, TEXT("Combat State being called. Current state is %d"), static_cast<int32>(CombatState));    OnRep_CombatState();
}

void UCombatComponent::OnRep_CombatState()
{
    if (!GetMainCharacter()) return;

    switch (CombatState)
    {
        case ECombatState::ECS_Reloading:
        if (!GetMainCharacter()->IsLocallyControlled())
        GetMainCharacter()->PlayReloadMontage();
        break;

        case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) Fire();

        default: break;
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

FVector_NetQuantize UCombatComponent::GetHitTarget() const
{
    if (!MainCharacter) return FVector::ZeroVector;

    // get the camera aim point
    FHitResult CamHit;
    TraceFromCamera(CamHit);
    const FVector AimPoint = CamHit.bBlockingHit ? CamHit.ImpactPoint : CamHit.TraceEnd;

    // ensure the muzzle can see that point; if not, use the obstruction
    if (!EquippedWeapon) return AimPoint;

    const FVector Muzzle = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(FName("Muzzle"));

    FHitResult MuzzleHit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MuzzleToAim), false);
    Params.AddIgnoredActor(MainCharacter);
    Params.AddIgnoredActor(EquippedWeapon);

    GetWorld()->LineTraceSingleByChannel(MuzzleHit, Muzzle, AimPoint, ECC_Visibility, Params);

    return MuzzleHit.bBlockingHit ? MuzzleHit.ImpactPoint : AimPoint;
}

void UCombatComponent::TraceFromCamera(FHitResult& HitResult) const
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



void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult) const
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

void UCombatComponent::TraceFromMuzzle(FHitResult& HitResult) const
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




