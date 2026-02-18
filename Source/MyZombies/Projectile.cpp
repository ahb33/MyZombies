// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Kismet/GameplayStatics.h" 
#include "Net/UnrealNetwork.h"
#include "Weapon.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"



// Sets default values
AProjectile::AProjectile()
{
    bReplicates = true;
    SetReplicateMovement(true);

    

    // Disable Tick as it's not needed
    PrimaryActorTick.bCanEverTick = false;

    // Create and configure the collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    SetRootComponent(CollisionSphere);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel1);
    CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

    // Bind the OnHit function
    CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
    // Create and configure the projectile movement component
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bShouldBounce = false;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f; // No gravity effect


    // Set default values
    ProjectileSpeed = 3000.0f;
    ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
    ProjectileMovementComponent->MaxSpeed = ProjectileSpeed;

    DamageAmount = 40.0f; // Example default damage
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("Projectile Spawned- Beginplay called"));

}
void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AProjectile, DamageAmount);
}


void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
    {
        return; // ignore self and owner
    }

    UE_LOG(LogTemp, Warning, TEXT("Projectile hit %s with damage: %f"), *OtherActor->GetName(), DamageAmount);

    // --- Server: apply damage + authoritative destroy ---
    if (HasAuthority())
    {
        UGameplayStatics::ApplyDamage(
            OtherActor,
            DamageAmount,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );

        // Turn off collision so we don't hit again before destroy
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // --- All machines: spawn cosmetics (FX/sound) ---
    if (currentWeapon)
    {
        if (UParticleSystem* ImpactParticles = currentWeapon->GetImpactParticles())
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                ImpactParticles,
                Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation()
            );
        }
    }

    // --- Destroy only if server ---
    if (HasAuthority())
    {
        Destroy();
    }
}


