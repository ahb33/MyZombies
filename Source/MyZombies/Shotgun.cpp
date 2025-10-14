// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Components/SceneComponent.h" 
#include "Camera/CameraComponent.h"
#include "Particles/ParticleSystem.h" 
#include "Kismet/GameplayStatics.h" 
#include "EnemyCharacter.h"
#include "Engine/World.h" 
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h" 

void AShotgun::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);
    if (!GetWorld() || !GetWeaponMesh()) return;

    const USkeletalMeshSocket* MuzzleSock =
        GetWeaponMesh()->GetSocketByName(AWeapon::GetMuzzleSocketName());
    if (!MuzzleSock) return;

    const FVector Start = MuzzleSock->GetSocketLocation(GetWeaponMesh());

    FVector AimDir = (HitTarget - Start).GetSafeNormal();
    if (AimDir.IsNearlyZero()) AimDir = GetActorForwardVector();

    const float Range   = GetMaxTraceDistance();
    const float ConeRad = FMath::DegreesToRadians(FMath::Max(ScatterAngle, 0.1f));
    FRandomStream Stream(FMath::Rand());

    for (int32 i = 0; i < NumPellets; ++i)
    {
        const FVector Dir = Stream.VRandCone(AimDir, ConeRad);
        const FVector End = Start + Dir * Range;

        FHitResult HR;
        const bool bHit = HitScanTrace(Start, End, HR);
        if (HasAuthority() && bHit) DealDamage(HR);
    }

    AWeapon::PlayFireEffects(FHitResult{});
}

void AShotgun::WeaponTraceWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutTargets)
{
    if (!GetWeaponMesh()) return;

    const USkeletalMeshSocket* MuzzleSock =
        GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (!MuzzleSock) return;

    const FVector M = MuzzleSock->GetSocketLocation(GetWeaponMesh());

    FVector Aim = (HitTarget - M).GetSafeNormal();
    if (Aim.IsNearlyZero()) {
        const FTransform Sx = MuzzleSock->GetSocketTransform(GetWeaponMesh());
        Aim = Sx.GetRotation().GetForwardVector();
        if (Aim.IsNearlyZero()) Aim = GetActorForwardVector();
    }

    const float ConeRad = FMath::DegreesToRadians(FMath::Max(ScatterAngle, 0.1f));
    const float Range   = GetMaxTraceDistance();

    OutTargets.Reset(NumPellets);
    for (int32 i = 0; i < NumPellets; ++i)
    {
        const FVector Dir = FMath::VRandCone(Aim, ConeRad);
        const FVector End = M + Dir * Range;

        FHitResult HR;
        if (HitScanTrace(M, End, HR))
            OutTargets.Add(HR.ImpactPoint);
        else
            OutTargets.Add(End);
    }
}