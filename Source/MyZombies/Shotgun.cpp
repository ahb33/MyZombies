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

    const USkeletalMeshSocket* MuzzleSock = GetWeaponMesh()->GetSocketByName(AWeapon::GetMuzzleSocketName());
    if (!MuzzleSock) return;

    const FVector Start = MuzzleSock->GetSocketLocation(GetWeaponMesh());

    // 1) Use your existing scatter builder
    TArray<FVector_NetQuantize> PelletTargets;
    WeaponTraceWithScatter(HitTarget, PelletTargets);

    // limit damage to 1 pellet
    const float PelletDamage = GetDamage() / FMath::Max(1, NumPellets);

    // Choose one pellet (closest hit or first miss) to drive FX
    FHitResult BestHR; 
    bool bHaveBest = false;
    FVector VisualEnd = Start;

    for (const FVector_NetQuantize& T : PelletTargets)
    {
        const FVector End = FVector(T);

        FHitResult HR;
        const bool bHit = HitScanTrace(Start, End, HR);

        if (HasAuthority() && bHit)
        {
            UGameplayStatics::ApplyDamage(
                HR.GetActor(),
                PelletDamage,
                GetOwner() ? GetOwner()->GetInstigatorController() : nullptr,
                this,
                UDamageType::StaticClass());
        }

        // Pick best visual ray (prefer a real impact)
        if (bHit)
        {
            const float DistSq = FVector::DistSquared(Start, HR.ImpactPoint);
            if (!bHaveBest || DistSq < FVector::DistSquared(Start, VisualEnd))
            {
                BestHR = HR; bHaveBest = true; VisualEnd = HR.ImpactPoint;
            }
        }
        else if (!bHaveBest)
        {
            VisualEnd = End; // fallback if nothing hits
        }
    }

    AWeapon::PlayFireEffects(bHaveBest ? BestHR : FHitResult{}, Start, VisualEnd);
}
void AShotgun::WeaponTraceWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutTargets)
{
    if (!GetWeaponMesh()) return;

    const USkeletalMeshSocket* MuzzleSock = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
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