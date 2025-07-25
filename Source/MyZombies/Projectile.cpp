// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Kismet/GameplayStatics.h" 
#include "Net/UnrealNetwork.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"

// move tracer, impact particles, and impact sound to weapon.cpp and access them in this class. 
// each weapon should set its own 

// Sets default values
AProjectile::AProjectile()
{
    // Disable Tick as it's not needed
    PrimaryActorTick.bCanEverTick = false;

    // Create and configure the collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    SetRootComponent(CollisionSphere);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel1);
    CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);


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

    DamageAmount = 10.0f; // Example default damage
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
    UE_LOG(LogTemp, Warning, TEXT("Projectile hit %s with damage: %f"), *OtherActor->GetName(), DamageAmount);

    // Ensure the projectile doesn't hit itself or its owner
    if (OtherActor && OtherActor != GetOwner())
    {
        if (HasAuthority()) // Only the server applies damage
        {
            UGameplayStatics::ApplyDamage(
                OtherActor,
                DamageAmount,
                GetInstigatorController(),
                this,
                UDamageType::StaticClass()
            );
        }

        // Disable collision before destroying
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // Spawn impact particles
        if (currentWeapon && currentWeapon->GetImpactParticles())
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                currentWeapon->GetImpactParticles(),
                Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation()
            );
        }

        // Play impact sound
        if (currentWeapon && currentWeapon->GetImpactSound())
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                currentWeapon->GetImpactSound(),
                Hit.ImpactPoint
            );
        }

        // Destroy the projectile after impact
        Destroy();
    }
}
void AProjectile::InitializeTracer()
{
    if (currentWeapon && currentWeapon->GetTracer())
    {
        TracerSystem = UGameplayStatics::SpawnEmitterAttached(currentWeapon->GetTracer(), CollisionSphere);
    }
}

