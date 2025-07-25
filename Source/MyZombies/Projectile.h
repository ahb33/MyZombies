// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h" 
#include "Sound/SoundCue.h"
#include "Projectile.generated.h"

UCLASS()
class MYZOMBIES_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

    FORCEINLINE float GetProjectileSpeed() const { return ProjectileSpeed; } // Getter for projectile speed

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

    UPROPERTY(VisibleAnywhere, Category = Projectile)
    class USphereComponent* CollisionSphere;

	/*We need to have some sort of hit event that will occur when projectile hits something
	we'd like to have function called in response to a hit event*/
    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        FVector NormalImpulse, const FHitResult& Hit);

	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }

    void SetProjectileDamage(float Damage) { DamageAmount = Damage; }
    void SetCurrentWeapon(class AWeapon* Weapon) { currentWeapon = Weapon; }
    void InitializeTracer();
    
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:	

    class AWeapon* currentWeapon;

    UPROPERTY(EditAnywhere, Category = Projectile)
    UParticleSystemComponent* TracerSystem;

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile|Movement")
	float ProjectileSpeed;

    UPROPERTY(Replicated, EditAnywhere, Category = "Projectile|Damage")
    float DamageAmount;

};
