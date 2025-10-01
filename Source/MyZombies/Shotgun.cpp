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

void AShotgun::Fire(const FVector& Hit)
{
    Super::Fire(Hit);       
    if (!GetOwner() || !GetWeaponMesh() ) return;

    const FVector M = GetWeaponMesh()->GetSocketLocation("Muzzle");
    const FVector Aim = (Hit - M).GetSafeNormal();
    const float Cone = FMath::DegreesToRadians(ScatterAngle);

    for (int32 i = 0; i < NumPellets; ++i)
    {
        const FVector Dir = FMath::VRandCone(Aim, Cone);
        FHitResult HR;
        if (HitScanTrace(M, M + Dir * TRACE_LENGTH, HR)) { DealDamage(HR); }
    }
    PlayFireEffects();
}

void AShotgun::WeaponTraceWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutTargets)
{
    if (!GetWeaponMesh()) return;

    const FName Muzzle("Muzzle");
    const FVector M = GetWeaponMesh()->GetSocketLocation(Muzzle);
    const FVector Aim = (HitTarget - M).GetSafeNormal();
    const float Cone = FMath::DegreesToRadians(ScatterAngle);

    OutTargets.Reset(NumPellets);
    for (int32 i = 0; i < NumPellets; ++i)
    {
        const FVector Dir = FMath::VRandCone(Aim, Cone);
        const FVector End = M + Dir * TRACE_LENGTH;

        // reuse same filtering as hitscan helper (no owner/self hits)
        FHitResult HR;
        if (HitScanTrace(M, End, HR))
            OutTargets.Add(HR.ImpactPoint);
        else
            OutTargets.Add(End);

        // optional debug:
        // DrawDebugLine(GetWorld(), M, HR.bBlockingHit ? HR.ImpactPoint : End, FColor::Green, false, 0.5f, 0, 0.5f);
    }
}

float AShotgun::GetDamage() const
{
    return 45.f;
}

