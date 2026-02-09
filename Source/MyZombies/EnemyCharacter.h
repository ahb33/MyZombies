// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AICharacterStats.h"
#include "AI_AnimInstance.h"
#include "GameplayTagAssetInterface.h" 
#include "GameplayTagContainer.h"
#include "EnemyCharacter.generated.h"


class AMyAIController;
class UBlackboardComponent;
class UAnimMontage;
class USphereComponent; 

USTRUCT(BlueprintType)
struct FPerceptionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) bool bSee = false;
    UPROPERTY(BlueprintReadOnly) bool bHear = false;
    UPROPERTY(BlueprintReadOnly) bool bInRange = false;
    UPROPERTY(BlueprintReadOnly) bool bChasing = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZombieDeath);

UCLASS(Abstract)
class MYZOMBIES_API AEnemyCharacter : public ACharacter , public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree.Get(); }
    bool CanSeePlayer() const { return PerceptionState.bSee; }
    bool CanHearPlayer() const { return PerceptionState.bHear; }
    bool IsPlayerWithinRange() const { return PerceptionState.bInRange; }
    bool IsChasing() const { return PerceptionState.bChasing; }

	void SetPerceptionState(bool bSee, bool bHear, bool bInRange);
    void SetChasingState(bool bChasing);


	UFUNCTION(BlueprintCallable, Category="Combat")
    virtual void Attack();

	UFUNCTION(Server, Reliable)
	void Server_Attack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(FName SectionName);

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& OutTags) const override
    {
        OutTags.AppendTags(CharacterTags);
    }

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Die();

	UFUNCTION()
	void OnRep_IsDead();
	UFUNCTION()
	void ApplyDeadState();

    // Functions to set multipliers or adjust properties based on stats
    void ApplyCharacterStats();

	// Delegate that other classes can bind to
    FOnZombieDeath OnZombieDeath;

	FORCEINLINE float GetWalkSpeed()  const { return WalkSpeed;  }
	FORCEINLINE float GetChaseSpeed() const { return ChaseSpeed; }
	FORCEINLINE float  GetAttackRange() const { return AttackRange; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:	

	// overlap
	TObjectPtr<USphereComponent> CollisionSphere = nullptr; 
	
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = Health, meta = (AllowPrivateAccess = "true"))
    float BaseHealth;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = Health, meta = (AllowPrivateAccess = "true"))
    float MaxHealth = 100;
	
	UPROPERTY(ReplicatedUsing=OnRep_IsDead)
	bool bIsDead = false;
	
	FTimerHandle DestructionTimer; // Timer handle for delayed destruction

	// AI Character stats for configuring properties in the editor
    FAICharacterStats CharacterStats;

	// Properties and functions that all enemies should have
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere, Category="Tags", meta=(AllowPrivateAccess="true"))
	FGameplayTagContainer CharacterTags;


	// EnemyCharacter.h (only the essentials)
	UPROPERTY(EditAnywhere, Category="Combat") float AttackRange    = 120.f;
	UPROPERTY(EditAnywhere, Category="Combat") float AttackDamage   = 20.f;
	UPROPERTY(EditAnywhere, Category="Combat") float AttackCooldown = 1.2f;
	UPROPERTY(EditAnywhere, Category="Combat") TObjectPtr<UAnimMontage> AttackMontage = nullptr;
	UPROPERTY(EditAnywhere, Category="AI|Blackboard") FName BBKey_Player = TEXT("Player");

	float NextAttackTime = 0.f;          // per-enemy cooldown

	UPROPERTY(EditAnywhere, Category="Movement") float WalkSpeed  = 120.f;
    UPROPERTY(EditAnywhere, Category="Movement") float ChaseSpeed = 360.f; // set your chase speed

	UPROPERTY(ReplicatedUsing=OnRep_PerceptionState)
	FPerceptionState PerceptionState;

	UFUNCTION()
	void OnRep_PerceptionState();


};
