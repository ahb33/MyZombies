// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AICharacterStats.h"
#include "AI_AnimInstance.h"
#include "EnemyCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZombieDeath);

UCLASS(Abstract)
class MYZOMBIES_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

	// Properties and functions that all enemies should have
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    virtual void Attack() PURE_VIRTUAL(AEnemyCharacter::Attack,);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION()
	void OnRep_Health();

    // Functions to set multipliers or adjust properties based on stats
    void ApplyCharacterStats();

	// Delegate that other classes can bind to
    FOnZombieDeath OnZombieDeath;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor); // function will be used to attack maincharater when overlapping with it

private:	

	// overlap
	class USphereComponent* CollisionSphere; 
	
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_Health)
    float BaseHealth;
	
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Health, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_Health)
    float BaseDamage;
	
	UAI_AnimInstance* AnimInstanceRef;

	FTimerHandle DestructionTimer; // Timer handle for delayed destruction

	class ABaseGameMode* myGameMode;

	// AI Character stats for configuring properties in the editor
    FAICharacterStats CharacterStats;

};
