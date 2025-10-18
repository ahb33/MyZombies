// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"


void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);
    if (!GetWorld() || !GetWeaponMesh()) return;

    const USkeletalMeshSocket* MuzzleSock = GetWeaponMesh()->GetSocketByName(AWeapon::GetMuzzleSocketName());
    if (!MuzzleSock) return;

    const FVector MuzzleLoc = MuzzleSock->GetSocketLocation(GetWeaponMesh());

    // Aim from muzzle toward the crosshair point; fallbacks preserved.
    FVector AimDir = (HitTarget - MuzzleLoc).GetSafeNormal();
    if (AimDir.IsNearlyZero())
    {
        AimDir = GetActorForwardVector();
        if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
        {
            if (const AController* Ctrl = PawnOwner->GetController())
            {
                const FVector CtrlDir = Ctrl->GetControlRotation().Vector();
                if (!CtrlDir.IsNearlyZero()) AimDir = CtrlDir;
            }
        }
    }

    const FVector Start = MuzzleLoc + AimDir * 2.f;                // avoid muzzle clip
    const FVector End   = Start + AimDir * GetMaxTraceDistance();  // ← never clamp to HitTarget

    FHitResult HR;
    const bool bHit = HitScanTrace(Start, End, HR);

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, bHit ? HR.ImpactPoint : End,
                  bHit ? FColor::Red : FColor::Green, false, 0.2f, 0, 0.75f);
    if (bHit) DrawDebugPoint(GetWorld(), HR.ImpactPoint, 6.f, FColor::Red, false, 1.f);
#endif

    if (HasAuthority() && bHit) DealDamage(HR);
    AWeapon::PlayFireEffects(HR, Start, bHit ? HR.ImpactPoint : End);
}


bool AHitScanWeapon::HitScanTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
    FCollisionQueryParams Params(SCENE_QUERY_STAT(HitScan), true);
    Params.bTraceComplex = true;                 // why: precise hits on skinned/thin geo
    Params.bReturnPhysicalMaterial = true;

    if (AActor* OwnerActor = GetOwner()) Params.AddIgnoredActor(OwnerActor);
    Params.AddIgnoredActor(this);
    if (USkeletalMeshComponent* WM = GetWeaponMesh()) Params.AddIgnoredComponent(WM);

    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}