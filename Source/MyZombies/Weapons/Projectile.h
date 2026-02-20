// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h" 
#include "Projectile.generated.h"


class USphereComponent;
class UProjectileMovementComponent;
class AWeapon;

UCLASS()
class MYZOMBIES_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

    FORCEINLINE float GetProjectileSpeed() const { return ProjectileSpeed; } // Getter for projectile speed

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;


	/*We need to have some sort of hit event that will occur when projectile hits something
	we'd like to have function called in response to a hit event*/
    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    FVector NormalImpulse, const FHitResult& Hit);

	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }
    FORCEINLINE USphereComponent* GetCollisionSphere() const { return CollisionSphere; }

    void SetProjectileDamage(float Damage) { DamageAmount = Damage; }
    void SetCurrentWeapon(class AWeapon* Weapon) { currentWeapon = Weapon; }
    bool bUseServerSideRewind = false;
    FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;
    
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:	

    TObjectPtr<AWeapon> currentWeapon = nullptr;

    UPROPERTY(VisibleAnywhere, Category = Projectile)
    TObjectPtr<USphereComponent> CollisionSphere = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Projectile|Movement")
	float ProjectileSpeed;

    UPROPERTY(Replicated, EditAnywhere, Category = "Projectile|Damage")
    float DamageAmount;

};
