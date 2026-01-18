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
#include "Camera/PlayerCameraManager.h"




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

    if (MainCharacter && MainCharacter->GetFollowCamera())
    {
        DefaultFOV = MainCharacter->GetFollowCamera()->FieldOfView;
        CurrentFOV = DefaultFOV;
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
    if (CombatState != ECombatState::ECS_Unoccupied || bIsReloading) return; 
    if (!bCanFire) return;

    bCanFire = false;

    switch (GetEquippedWeapon()->GetWeaponType())
    {
        case EWeaponType::AssaultRifle: FireHitScanWeapon(); 
        break;
        case EWeaponType::Shotgun:      FireShotgun();       
        break;
        default: break;
    }

    // (re)start ROF cooldown
    if (UWorld* W = GetWorld())
    {
        W->GetTimerManager().SetTimer(
            FireTimerHandle, this, &UCombatComponent::OnFireCooldownFinished,
            GetEquippedWeapon()->GetFireDelay(), false);
    }
}

void UCombatComponent::OnFireCooldownFinished()
{
    bCanFire = true;
    if (bFireButtonPressed && GetCombatState() == ECombatState::ECS_Unoccupied && !bIsReloading)
        Fire();
}


void UCombatComponent::PerformLocalAndServerFire(const FVector_NetQuantize& Target, void (UCombatComponent::*LocalFunc)(const FVector_NetQuantize&),
                                                                                    void (UCombatComponent::*ServerFunc)(const FVector_NetQuantize&))
{
    if (!GetMainCharacter()) return;
    if (!GetMainCharacter()->HasAuthority()) (this->*LocalFunc)(Target);
    (this->*ServerFunc)(Target);
}

void UCombatComponent::FireHitScanWeapon()
{
    if (!GetMainCharacter() || !Cast<AHitScanWeapon>(GetEquippedWeapon())) return;
    const FVector_NetQuantize NetTarget = BuildNetHitTargetFromController();
    PerformLocalAndServerFire(NetTarget, &UCombatComponent::HitScanLocalFire, &UCombatComponent::ServerHitScanFire);
}

void UCombatComponent::FireShotgun()
{
    if (!GetMainCharacter() || !Cast<AShotgun>(GetEquippedWeapon())) return;
    const FVector_NetQuantize NetTarget = BuildNetHitTargetFromController();
    PerformLocalAndServerFire(NetTarget, &UCombatComponent::ShotgunLocalFire, &UCombatComponent::ServerShotgunFire);
}

// Local cosmetic still calls weapon->Fire; weapon ignores the param now (safe).
void UCombatComponent::HitScanLocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (GetMainCharacter() && GetCombatState() == ECombatState::ECS_Unoccupied)
        if (AWeapon* W = GetEquippedWeapon()) W->Fire(TraceHitTarget);
}


void UCombatComponent::ShotgunLocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (AShotgun* Shotgun = Cast<AShotgun>(GetEquippedWeapon()))
    {
        bIsReloading = false;
        if (GetMainCharacter()) GetMainCharacter()->PlayFireMontage(bAiming);
        Shotgun->Fire(TraceHitTarget);
        SetCombatState(ECombatState::ECS_Unoccupied);
    }
}


void UCombatComponent::ServerHitScanFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    MulticastFireHitScan(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{   
    MulticastFireShotgun(TraceHitTarget);
}

bool UCombatComponent::ShouldSkipLocalMulticast() const
{
    return GetMainCharacter() && GetMainCharacter()->IsLocallyControlled() && !MainCharacter->HasAuthority();
}


void UCombatComponent::MulticastFireHitScan_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (ShouldSkipLocalMulticast()) return;
    HitScanLocalFire(TraceHitTarget);
}

void UCombatComponent::MulticastFireShotgun_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (ShouldSkipLocalMulticast()) return;
    ShotgunLocalFire(TraceHitTarget);
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (!GetMainCharacter() || !WeaponToEquip) return;

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        // Client → ask server
        ServerEquipWeapon(WeaponToEquip);
        return;
    }

    // Server logic
    if (GetEquippedWeapon() && !GetSecondaryWeapon())
    {
        EquipSecondaryWeapon(WeaponToEquip);
    }
    else
    {
        EquipPrimaryWeapon(WeaponToEquip);
    }
}

void UCombatComponent::ApplyPrimaryEquipSetup(AWeapon* WeaponToEquip)
{
    EquippedWeapon = WeaponToEquip;
    GetEquippedWeapon()->SetWeaponState(EWeaponState::EWS_Equipped);
    if (GetMainCharacter()) GetEquippedWeapon()->SetOwner(GetMainCharacter());
    AttachActorToRightHand(GetEquippedWeapon());
    GetEquippedWeapon()->ShowPickUpWidget(false);

    if (GetMainCharacter() && GetMainCharacter()->IsLocallyControlled()) GetEquippedWeapon()->RefreshHUD();

    if (GetMainCharacter())
    {
        GetMainCharacter()->GetCharacterMovement()->bOrientRotationToMovement = false;
        GetMainCharacter()->bUseControllerRotationYaw = true;
    }
}


void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
    if (!WeaponToEquip || !GetMainCharacter()) return;
    ApplyPrimaryEquipSetup(WeaponToEquip);
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
    if (!WeaponToEquip || !GetMainCharacter()) return;
    SecondaryWeapon = WeaponToEquip;
    GetSecondaryWeapon()->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
    GetSecondaryWeapon()->SetOwner(GetMainCharacter());
    AttachWeaponToBackSocket(GetSecondaryWeapon());
    GetSecondaryWeapon()->ShowPickUpWidget(false);
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
    if (!GetEquippedWeapon() || !GetSecondaryWeapon()) return;

    // Swap primary and secondary
    Swap(EquippedWeapon, SecondaryWeapon);

    // Attach new primary weapon to right hand
    AttachActorToRightHand(GetEquippedWeapon());
    GetEquippedWeapon()->SetOwner(GetMainCharacter());
    GetEquippedWeapon()->SetWeaponState(EWeaponState::EWS_Equipped);

    // Attach new secondary weapon to back
    AttachWeaponToBackSocket(GetSecondaryWeapon());
    GetSecondaryWeapon()->SetOwner(GetMainCharacter());
    GetSecondaryWeapon()->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

    GetEquippedWeapon()->RefreshHUD();

}

bool UCombatComponent::ShouldSwapWeapons() const
{
    return GetEquippedWeapon() && GetSecondaryWeapon();
}


void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
    if (!GetMainCharacter() || !ActorToAttach) return;

    const USkeletalMeshSocket* HandSocket = GetMainCharacter()->GetMesh()->GetSocketByName(FName("RightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(ActorToAttach, GetMainCharacter()->GetMesh());
    }
}

void UCombatComponent::AttachWeaponToBackSocket(AActor* WeaponToAttach)
{
	// check if character or weapon to attach are nullptr
	// create and define const ref to USkeletalMeshSocket 
	// check if ref is valid
	// AttachActor 
    if (!GetMainCharacter() || !GetMainCharacter()->GetMesh() || !WeaponToAttach) return;

    const USkeletalMeshSocket* WeaponSocket = GetMainCharacter()->GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket"));
    if (WeaponSocket)
    {
        WeaponSocket->AttachActor(WeaponToAttach, GetMainCharacter()->GetMesh());
    }
}

void UCombatComponent::Reload()
{
    if (!GetEquippedWeapon() || bIsReloading || GetCombatState() != ECombatState::ECS_Unoccupied) return;

    if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(FireTimerHandle);

    bCanFire = false;
    bIsReloading = true;
    ServerReload();

    if (GetMainCharacter() && GetMainCharacter()->IsLocallyControlled()) GetMainCharacter()->PlayReloadMontage();
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

    float ReloadDuration = GetMainCharacter()->GetReloadDuration();
    
    // UE_LOG(LogTemp, Warning, TEXT("ReloadTimer = %.2f, MontageLen = %.2f"),ReloadDuration,MainCharacter->GetReloadMontage() ? MainCharacter->GetReloadMontage()->GetPlayLength() : 0.f);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ReloadTimerHandle);
        World->GetTimerManager().SetTimer(
            ReloadTimerHandle, this, &UCombatComponent::FinishReloading, ReloadDuration, false);
    }
}

void UCombatComponent::FinishReloading()
{
    if (!GetEquippedWeapon()) return;
    bIsReloading = false;

    if (GetMainCharacter() && GetMainCharacter()->HasAuthority())
    {
        SetCombatState(ECombatState::ECS_Unoccupied);
        bCanFire = true;
        if (bFireButtonPressed) Fire(); // server-only autoresume
    }
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
    if (!GetMainCharacter() || !GetEquippedWeapon()) return;
    if (!GetMainCharacter()->HasAuthority()) ServerSetAiming(bIsAimingIn); // if you dont have authority call ServerSetAiming
    bAiming = bIsAimingIn;
}


void UCombatComponent::ServerSetAiming_Implementation(bool bIsAimingIn)
{
    bAiming = bIsAimingIn;
}



void UCombatComponent::SetCombatState(ECombatState State)
{
    CombatState = State;
}

void UCombatComponent::OnRep_CombatState()
{
    if (!GetMainCharacter()) return;

    switch (GetCombatState())
    {
        case ECombatState::ECS_Reloading:
            bIsReloading = true;
            bCanFire = false;
            if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(FireTimerHandle);
            if (!GetMainCharacter()->IsLocallyControlled()) GetMainCharacter()->PlayReloadMontage();
            break;

        case ECombatState::ECS_Unoccupied:
            bIsReloading = false;
            bCanFire = true;
            break;

        default: break;
    }
}


void UCombatComponent::SetZooming(bool bIsZooming)
{
    bZooming = bIsZooming;
}


void UCombatComponent::HandleZoom(float DeltaTime)
{
    if (!GetEquippedWeapon()) return;

    if (bAiming)
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV,GetEquippedWeapon()->GetZoomedFOV(), DeltaTime, GetEquippedWeapon()->GetZoomInterpSpeed());
    }
    else
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
    }

    if (GetMainCharacter() && GetMainCharacter()->GetFollowCamera())
    {
        GetMainCharacter()->GetFollowCamera()->SetFieldOfView(CurrentFOV);
    }
}


void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
    if (!GetMainCharacter()) return;

    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetMainCharacter()->GetController());
    if (!PlayerController) return;

    HUD = HUD ? HUD : Cast<AMyHUD>(PlayerController->GetHUD());
    if (!HUD) return;

    HUDPackage.CrosshairsCenter = CrosshairsCenter;
    HUDPackage.CrosshairsLeft   = CrosshairsLeft;
    HUDPackage.CrosshairsRight  = CrosshairsRight;
    HUDPackage.CrosshairsTop    = CrosshairsTop;
    HUDPackage.CrosshairsBottom = CrosshairsBottom;

    FVector2D SpeedRange(0.f, GetMainCharacter()->GetCharacterMovement()->MaxWalkSpeed);
    FVector2D VelocityRange(0.f, 1.f);
    FVector Velocity = GetMainCharacter()->GetVelocity();
    Velocity.Z = 0.f;

    CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(SpeedRange, VelocityRange, Velocity.Size());
    HUDPackage.CrosshairSpread = 0.0f + CrosshairVelocityFactor;

    HUD->SetHUDPackage(HUDPackage);
}


static constexpr float kCamNudge = 16.f;
static constexpr float kMuzzleNudge = 2.f;
static constexpr float kMinValidCamSq = 25.f * 25.f; // treat ultra-near hits as camera clipping

// Stable camera view helper (no debug, no extra classes)
static void GetStableCameraView(const AMainCharacter* MC, FVector& OutLoc, FRotator& OutRot)
{
	OutLoc = FVector::ZeroVector;
	OutRot = FRotator::ZeroRotator;
	if (!MC) return;

	if (const APlayerController* PC = Cast<APlayerController>(MC->GetController()))
	{
		if (const APlayerCameraManager* PCM = PC->PlayerCameraManager)
		{
			OutLoc = PCM->GetCameraLocation();
			OutRot = PCM->GetCameraRotation();
			return;
		}
		PC->GetPlayerViewPoint(OutLoc, OutRot);
	}
}

static FCollisionQueryParams MakeTraceParams(const AMainCharacter* MC, const AWeapon* W, FName StatName)
{
    FCollisionQueryParams Params(StatName, true);
    Params.bReturnPhysicalMaterial = true; // why: surface-based FX/damage

    // why: centralize to avoid missing either actor or mesh in future
    auto IgnoreWeapon = [&Params](const AActor* WeaponActor)
    {
        if (!WeaponActor) return;
        Params.AddIgnoredActor(WeaponActor);

        // Prefer const-safe lookup over non-const getters
        if (const USkeletalMeshComponent* Mesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>())
        {
            Params.AddIgnoredComponent(Mesh);
        }
    };

    if (MC)
    {
        Params.AddIgnoredActor(MC); // why: avoid self-hits near geometry

        TArray<AActor*> Attached;
        MC->GetAttachedActors(Attached);
        for (const AActor* A : Attached)
        {
            if (Cast<AWeapon>(A)) // only ignore weapons among attachments
            {
                IgnoreWeapon(A);
            }
        }
    }

    IgnoreWeapon(W);

    return Params;
}

FVector_NetQuantize UCombatComponent::BuildNetHitTargetFromController() const
{
    if (!GetMainCharacter() || !GetEquippedWeapon()) return FVector::ZeroVector;

    const float Range = GetEquippedWeapon()->GetMaxTraceDistance() > 0.f ? GetEquippedWeapon()->GetMaxTraceDistance(): TRACE_LENGTH;

    // Crosshair world ray
    FHitResult XR;
    TraceUnderCrosshairs(XR);

    // Stable camera for fallbacks / micro-hit rejection
    FVector CamLoc; FRotator CamRot;
    GetStableCameraView(GetMainCharacter(), CamLoc, CamRot);

    if (XR.bBlockingHit)
    {
        // why: avoid camera clip hits (too close)
        if (FVector::DistSquared(XR.ImpactPoint, CamLoc) >= kMinValidCamSq)
            return XR.ImpactPoint;

        return XR.TraceEnd.IsNearlyZero() ? CamLoc + CamRot.Vector() * Range : XR.TraceEnd;
    }

    // No hit → use far end of crosshair ray
    if (!XR.TraceEnd.IsNearlyZero())
        return XR.TraceEnd;

    return CamLoc + CamRot.Vector() * Range;
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& OutHit) const
{
    OutHit = FHitResult();
    if (!GetMainCharacter() || !GetWorld()) return;

    if (APlayerController* PC = Cast<APlayerController>(GetMainCharacter()->GetController()))
    {
        int32 SizeX = 0, SizeY = 0;
        PC->GetViewportSize(SizeX, SizeY);
        if (SizeX <= 0 || SizeY <= 0) return;

        const FVector2D ScreenPos(SizeX * 0.5f, SizeY * 0.5f);

        FVector WorldPos, WorldDir;
        if (UGameplayStatics::DeprojectScreenToWorld(PC, ScreenPos, WorldPos, WorldDir))
        {
            const FVector Start = WorldPos + WorldDir * kCamNudge;
            const FVector End   = Start + WorldDir * TRACE_LENGTH;

            FCollisionQueryParams Params = MakeTraceParams(GetMainCharacter(), GetEquippedWeapon(), TEXT("TraceUnderCrosshairs"));
            GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
        }
    }
}
