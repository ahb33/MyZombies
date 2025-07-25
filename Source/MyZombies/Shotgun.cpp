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


void AShotgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AShotgun, ShotgunAmmoOnHand, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AShotgun, ShotgunAmmoInMag, COND_OwnerOnly);
    DOREPLIFETIME(AShotgun, ShotgunMaxAmmoOnHand);
    DOREPLIFETIME(AShotgun, ShotgunMagCapacity);
}



void AShotgun::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    UWorld* World = GetWorld();

    if (MuzzleFlashSocket && World)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        FVector MuzzleLocation = SocketTransform.GetLocation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = InstigatorPawn;

        // Number of pellets for the shotgun
        const int32 NumPellets = 10;

        for (int32 i = 0; i < NumPellets; ++i)
        {
            // Calculate a scatter endpoint for each pellet
            FVector ScatterEndPoint = CalculateScatterEndPoint(MuzzleLocation, HitTarget);
            UE_LOG(LogTemp, Warning, TEXT("ScatterEndPoint = %s"), *ScatterEndPoint.ToString());

            // Perform a trace to find the actual hit location
            FHitResult FireHit;
            WeaponTraceWithScatter(MuzzleLocation, ScatterEndPoint, FireHit);

            // Spawn the projectile for the pellet
            if (InstigatorPawn && InstigatorPawn->HasAuthority())
            {
                AProjectile* SpawnedProjectile = World->SpawnActor<AProjectile>(
                    Projectile,
                    MuzzleLocation,
                    (ScatterEndPoint - MuzzleLocation).Rotation(),
                    SpawnParams
                );

                if (SpawnedProjectile)
                {
                    SpawnedProjectile->SetProjectileDamage(GetDamage() / NumPellets); // Split damage across pellets
                    SpawnedProjectile->SetCurrentWeapon(this);
                }
            }
        }
    }
}

void AShotgun::WeaponTraceWithScatter(const FVector &TraceStart, const FVector &EndPoint, FHitResult &FireHit)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->LineTraceSingleByChannel(
            FireHit,
            TraceStart,
            EndPoint,
            ECollisionChannel::ECC_Visibility
        );

        // Optional: Debug visualization for the trace
#if WITH_EDITOR
        FColor DebugColor = FireHit.bBlockingHit ? FColor::Red : FColor::Green;
        DrawDebugLine(World, TraceStart, EndPoint, DebugColor, false, 1.f, 0, 1.f);
#endif
    }
}

FVector AShotgun::CalculateScatterEndPoint(const FVector& MuzzleLocation, const FVector& HitTarget)
{
    // Angle of scatter in degrees
    float ScatterAngle = 5.0f;

    // Normalize shot direction
    FVector ShotDirection = (HitTarget - MuzzleLocation).GetSafeNormal();

    // Randomize shot direction within a cone
    FVector ScatterDirection = FMath::VRandCone(ShotDirection, FMath::DegreesToRadians(ScatterAngle));

    // Scale direction to the trace length
    return MuzzleLocation + ScatterDirection * TRACE_LENGTH; // Use your defined TRACE_LENGTH
}


void AShotgun::SetAmmo(int32 NewAmmoOnHand, int32 NewAmmoInMag)
{
    ShotgunAmmoOnHand = NewAmmoOnHand;
    ShotgunAmmoInMag = NewAmmoInMag;

    // // Now also update the base class's ammo count
    // AWeapon::SetAmmoOnHand(NewAmmoOnHand);

}

int32 AShotgun::GetCurrentAmmoOnHand() const
{
    return ShotgunAmmoOnHand;
}

int32 AShotgun::GetCurrentAmmoInMag() const
{
    return ShotgunAmmoInMag;
}

int32 AShotgun::GetMaxAmmoOnHand() const
{
    return ShotgunMaxAmmoOnHand;
}

int32 AShotgun::GetMagCapacity() const
{
    return ShotgunMagCapacity;
}

void AShotgun::ReloadAmmo(int32 AmmoAmount) 
{
// Update the Shotgun-specific ammo counts

    ShotgunAmmoOnHand += AmmoAmount;
    ShotgunAmmoInMag -= AmmoAmount;

    // Ensure the values stay within valid bounds
    ShotgunAmmoOnHand = FMath::Clamp(ShotgunAmmoOnHand, 0, ShotgunMaxAmmoOnHand);
    ShotgunAmmoInMag = FMath::Max(ShotgunAmmoInMag, 0);

}


float AShotgun::GetDamage() const
{
    return 45.f;
}

void AShotgun::OnRep_ShotgunAmmoOnHand()
{
    RefreshHUD();
}

void AShotgun::OnRep_ShotgunAmmoInMag()
{
    RefreshHUD();
}

