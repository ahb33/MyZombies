// Fill out your copyright notice in the Description page of Project Settings.


#include "AssaultWeapon.h"
#include "Components/SceneComponent.h" 
#include "Engine/World.h" 
#include "Engine/SkeletalMeshSocket.h" 
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "EnemyCharacter.h"


void AAssaultWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    UWorld* World = GetWorld();

    if (MuzzleFlashSocket && World)
    {
        // Get the transform of the muzzle socket
        SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

        // Compute direction to target
        FVector ToTarget = HitTarget - SocketTransform.GetLocation();
        FRotator TargetRotation = ToTarget.Rotation();

        // Setup spawn parameters
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = InstigatorPawn;

        if (InstigatorPawn && InstigatorPawn->HasAuthority()) // Only server spawns the projectile
        {
            AProjectile* SpawnedProjectile = World->SpawnActor<AProjectile>(
                Projectile,                     // Projectile class
                SocketTransform.GetLocation(),  // Spawn location
                TargetRotation,                 // Spawn rotation
                SpawnParams                     // Spawn parameters
            );

            if (SpawnedProjectile)
            {
                SpawnedProjectile->SetProjectileDamage(GetDamage()); // Pass weapon damage to the projectile
                UE_LOG(LogTemp, Warning, TEXT("Setting Current Weapon"));
                SpawnedProjectile->SetCurrentWeapon(this);
                SpawnedProjectile->InitializeTracer();  // Trigger tracer setup after weapon is set
            }
        }
    }
}

void AAssaultWeapon::SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag)
{
    AssaultWeaponAmmoOnHand = NewAmmoOnHand;
    AssaultWeaponAmmoInMag = NewAmmoInMag;

    // AWeapon::SetAmmoOnHand(NewAmmoOnHand);

}
int32 AAssaultWeapon::GetCurrentAmmoOnHand() const
{
    return AssaultWeaponAmmoOnHand;
}

int32 AAssaultWeapon::GetCurrentAmmoInMag() const
{
    return AssaultWeaponAmmoInMag;
}

int32 AAssaultWeapon::GetMaxAmmoOnHand() const
{
    return AssaultWeaponMaxAmmoOnHand;
}

int32 AAssaultWeapon::GetMagCapacity() const
{
    return AssaultWeaponMagCapacity;
}

void AAssaultWeapon::ReloadAmmo(int32 AmmoAmount) 
{
    // Update the AssaultWeapon-specific ammo counts
    AssaultWeaponAmmoOnHand += AmmoAmount;
    AssaultWeaponAmmoInMag -= AmmoAmount;

    // Ensure the values stay within valid bounds
    AssaultWeaponAmmoOnHand = FMath::Clamp(AssaultWeaponAmmoOnHand, 0, AssaultWeaponMaxAmmoOnHand);
    AssaultWeaponAmmoInMag = FMath::Max(AssaultWeaponAmmoInMag, 0);

}




