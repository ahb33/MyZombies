// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Components/SceneComponent.h" 
#include "Engine/World.h" 
#include "Engine/SkeletalMeshSocket.h" 
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Perception/AISense_Hearing.h"
#include "EnemyCharacter.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);


    APawn* InstigatorPawn = Cast<APawn>(GetOwner());
    if (!InstigatorPawn || !GetWeaponMesh())
    {
        UE_LOG(LogTemp, Error, TEXT("Fire aborted: InstigatorPawn=%s, WeaponMesh=%s"),
            InstigatorPawn ? *InstigatorPawn->GetName() : TEXT("NULL"),
            GetWeaponMesh() ? *GetWeaponMesh()->GetName() : TEXT("NULL"));
        return;
    }

    if (!HasAuthority())
    {
    UE_LOG(LogTemp, Warning, TEXT("Skipping projectile spawn on client (cosmetic handled elsewhere)"));
    return;
    }

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (!MuzzleFlashSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Fire aborted: No MuzzleFlash socket on %s"), *GetWeaponMesh()->GetName());
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Fire aborted: World is NULL"));
        return;
    }

    SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    FVector ToTarget = HitTarget - SocketTransform.GetLocation();
    FRotator TargetRotation = ToTarget.Rotation();

    UE_LOG(LogTemp, Warning, TEXT("AProjectileWeapon::Fire | NetMode=%d | Authority=%d | LocallyControlled=%d"),
    (int32)World->GetNetMode(),
    InstigatorPawn->HasAuthority(),
    InstigatorPawn->IsLocallyControlled());

    // Server-side spawn params
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = InstigatorPawn;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Cosmetic spawn params (clients)
    FActorSpawnParameters CosmeticParams;
    CosmeticParams.Owner = nullptr;
    CosmeticParams.Instigator = nullptr;
    CosmeticParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AProjectile* SpawnedProjectile = nullptr;

    if (bUseServerSideRewind)
    {
        if (InstigatorPawn->HasAuthority())
        {
            if (InstigatorPawn->IsLocallyControlled())
            {
               UE_LOG(LogTemp, Warning, TEXT("Branch: Listen server host firing own weapon"));
                SpawnedProjectile = World->SpawnActor<AProjectile>(
                    ProjectileClass,
                    SocketTransform.GetLocation(),
                    TargetRotation,
                    SpawnParams
                );

                if (SpawnedProjectile)
                {
                    SpawnedProjectile->bUseServerSideRewind = false;
                    SpawnedProjectile->SetProjectileDamage(GetDamage());
                    SpawnedProjectile->SetCurrentWeapon(this);
                    // SpawnedProjectile->InitializeTracer();
                }
            }

            // server, not locally controlled
            else 
            {
                UE_LOG(LogTemp, Warning, TEXT("Branch: Dedicated server simulating for remote client"));
                SpawnedProjectile = World->SpawnActor<AProjectile>(
                    ServerSideRewindProjectileClass,
                    SocketTransform.GetLocation(),
                    TargetRotation,
                    SpawnParams
                );

                if (SpawnedProjectile)
                {
                    SpawnedProjectile->bUseServerSideRewind = true;
                    SpawnedProjectile->SetCurrentWeapon(this);
                }
            }
        }

        // client using SSR
        else 
        {
            // We are on a client
            if (InstigatorPawn->IsLocallyControlled())
            {
                UE_LOG(LogTemp, Warning, TEXT("Branch: Client firing own shot"));

                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(), 
                TargetRotation, CosmeticParams);

                if (SpawnedProjectile)
                {
                    SpawnedProjectile->bUseServerSideRewind = true;
                    SpawnedProjectile->TraceStart = SocketTransform.GetLocation();

                    if (auto* MoveComp = SpawnedProjectile->GetProjectileMovementComponent())
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Move Comp successfully retreived"));
                        SpawnedProjectile->InitialVelocity =
                        SpawnedProjectile->GetActorForwardVector() * MoveComp->InitialSpeed;
                    }

                    // SpawnedProjectile->SetCurrentWeapon(this);
                }
            }
        }
    }
    else
    {
        if (InstigatorPawn->HasAuthority())
        {
            UE_LOG(LogTemp, Warning, TEXT("Branch: No SSR, server authoritative projectile"));

			SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			SpawnedProjectile->bUseServerSideRewind = false;
        }
    }
    
    // Post-spawn log
    if (SpawnedProjectile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Spawned projectile of class %s | Location=%s | bSSR=%d"),
        *SpawnedProjectile->GetClass()->GetName(),
        *SpawnedProjectile->GetActorLocation().ToString(),
        SpawnedProjectile->bUseServerSideRewind);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnActor failed in branch! Class(Projectile)=%s | Class(SSR)=%s"),
        ProjectileClass ? *ProjectileClass->GetName() : TEXT("NULL"),
        ServerSideRewindProjectileClass ? *ServerSideRewindProjectileClass->GetName() : TEXT("NULL"));
    }
}




