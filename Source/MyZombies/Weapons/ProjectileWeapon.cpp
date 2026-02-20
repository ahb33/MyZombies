// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Components/SceneComponent.h" 
#include "Engine/World.h" 
#include "Engine/SkeletalMeshSocket.h" 
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Perception/AISense_Hearing.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);


    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Skipping projectile spawn on client (cosmetic handled elsewhere)"));
        return;
    }

    const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (!MuzzleSocket || !GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Fire aborted: No muzzle socket or world"));
        return;
    }

    const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
    const FRotator TargetRotation = (HitTarget - SocketTransform.GetLocation()).Rotation();

    TSubclassOf<AProjectile> ChosenClass = bUseServerSideRewind 
        ? ServerSideRewindProjectileClass 
        : ProjectileClass;

    if (!ChosenClass)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid projectile class (SSR=%d)"), bUseServerSideRewind);
        return;
    }
    // Server-side spawn params
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 
    AProjectile* Spawned = GetWorld()->SpawnActor<AProjectile>(
        ChosenClass,
        SocketTransform.GetLocation(),
        TargetRotation,
        SpawnParams
    );

    if (Spawned)
    {
        Spawned->bUseServerSideRewind = bUseServerSideRewind;
        Spawned->SetProjectileDamage(GetDamage());
        Spawned->SetCurrentWeapon(this);

        if (bUseServerSideRewind)
        {
            Spawned->TraceStart = SocketTransform.GetLocation();
            if (auto* MoveComp = Spawned->GetProjectileMovementComponent())
            {
                Spawned->InitialVelocity = Spawned->GetActorForwardVector() * MoveComp->InitialSpeed;
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnActor failed for %s"), *ChosenClass->GetName());
    }
}

 