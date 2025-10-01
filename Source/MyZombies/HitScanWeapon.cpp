// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"



void AHitScanWeapon::Fire(const FVector &Hit)
{
    Super::Fire(Hit);

    // if (!GetOwner() || !GetWeaponMesh()) return;

    const FName MuzzleName("Muzzle");
    const FVector Start = GetWeaponMesh()->GetSocketLocation(MuzzleName);
    const FVector Dir   = (Hit - Start).GetSafeNormal();
    const FVector End   = Start + Dir * TRACE_LENGTH;

    FHitResult HR;
    if (HitScanTrace(Start, End, HR))
    {
        DealDamage(HR);
        #if WITH_EDITOR
        DrawDebugPoint(GetWorld(), HR.ImpactPoint, 6.f, FColor::Red, false, 1.f);
        DrawDebugLine(GetWorld(), Start, HR.ImpactPoint, FColor::Red, false, 0.2f, 0, 0.5f);
        #endif
    }
    else
    {
        #if WITH_EDITOR
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.2f, 0, 0.5f);
        #endif
    }

    PlayFireEffects();                                  // muzzle/tracer
}

bool AHitScanWeapon::HitScanTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
    FCollisionQueryParams Params(SCENE_QUERY_STAT(HitScan), false);
    Params.AddIgnoredActor(GetOwner());
    Params.AddIgnoredActor(this);
    UWorld* World = GetWorld();
    return World && World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}



