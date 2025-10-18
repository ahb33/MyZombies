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

	// virtual float ApplyDamage;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_IsDead();

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

	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;
	
	UAI_AnimInstance* AnimInstanceRef;

	FTimerHandle DestructionTimer; // Timer handle for delayed destruction

	// AI Character stats for configuring properties in the editor
    FAICharacterStats CharacterStats;

/*
================================================================================
AI MELEE: SPEC & EXECUTION PLAN (review-ready)
--------------------------------------------------------------------------------
Goal:
- Deterministic, animation-synced melee hit that damages the player exactly once
  per swing, with clean cooldown and range validation.

Scope:
- Single enemy type. No combo system. No root motion handling changes.

Authoritative rules:
- Damage authority = Server only. Clients play anims/VFX only.
- Hit frame triggered by Anim Notify → calls AnimNotify_MeleeHit().

Blackboard/Context:
- BB key "Player" (Actor) must be set by AIController/Perception.
- Attack runs only if target is within AttackRange at start AND at hit frame.

Flow (per attack attempt):
1) Attack() called (BT Task / AIController decision).
2) Preconditions:
   - !bIsDead
   - Now >= NextAttackTime (cooldown)
   - Target resolved (BB "Player" else Player0)
   - DistanceSq <= AttackRange^2
3) Start montage (AttackMontage). Set:
   - bHitAppliedThisSwing = false
   - NextAttackTime = Now + AttackCooldown
   - Cache target weak ptr (avoid GC issues)
4) At notify (AnimNotify_MeleeHit):
   - If bHitAppliedThisSwing == false:
       - Re-resolve target (cache fallback)
       - Re-validate range (lag-safe)
       - Server: UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass())
       - bHitAppliedThisSwing = true
5) Exit: montage end/no-op; BT can wait for cooldown externally if desired.

Networking:
- Damage only on server (HasAuthority() check inside AnimNotify_MeleeHit()).
- Optional cosmetic multicast (SFX/impact) if needed later.

Tuning knobs (UPROPERTY EditAnywhere):
- AttackRange (cm), AttackDamage, AttackCooldown (s), AttackMontage.

Debug & Telemetry:
- Optional DEV_LOG(AI, "Attack start/hit; dist=%.1f") guards.
- GameplayDebugger: show AttackRange sphere on request.

Edge cases:
- Target dies/despawns mid-swing → skip damage.
- Lost sight but still in range at notify → still valid (melee is tactile).
- Multiple notifies in montage → guard via bHitAppliedThisSwing.
- Montage interrupted → no damage unless notify already fired.

Performance:
- O(1) per call; single distance check & one ApplyDamage per swing. No traces.

Risks:
- Notify timing mismatch → fix in AnimMontage by moving notify to contact frame.
================================================================================
*/

protected:

	UFUNCTION(BlueprintCallable, Category="Combat")
    virtual void Attack() override;

	UFUNCTION(BlueprintCallable, Category="Combat")
    void AnimNotify_MeleeHit(); // use a notify that applies damage at the hit frame


};
