// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "MyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DamageHelpers.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

static FORCEINLINE AActor* GetBBTarget(const AAIController* AIC, FName Key)
{
    if (!AIC) return nullptr;
    if (const UBlackboardComponent* BB = AIC->GetBlackboardComponent())
    {
        return Cast<AActor>(BB->GetValueAsObject(Key));
    }
    return nullptr;
}

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
    bReplicates = true;
    SetReplicateMovement(true); 
	PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetSphereRadius(50);
    CollisionSphere->SetupAttachment(RootComponent);

    CharacterTags.AddTag(FGameplayTag::RequestGameplayTag("Faction.Zombie"));


}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    const FString Diff = GetWorld() ? FString(GetWorld()->URL.GetOption(TEXT("Difficulty="), TEXT("Medium"))) : TEXT("Medium");

    float HealthMult = 1.0f, SpeedMult = 1.0f, DamageMult = 1.0f;
    if (Diff.Equals(TEXT("Easy"), ESearchCase::IgnoreCase))      { HealthMult = 0.8f; SpeedMult = 0.9f; DamageMult = 0.8f; }
    else if (Diff.Equals(TEXT("Hard"), ESearchCase::IgnoreCase)) { HealthMult = 1.3f; SpeedMult = 1.1f; DamageMult = 1.2f; }

    MaxHealth    *= HealthMult;
    AttackDamage *= DamageMult;
    WalkSpeed    *= SpeedMult;
    ChaseSpeed   *= SpeedMult;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = WalkSpeed;
    }

    BaseHealth = MaxHealth;
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEnemyCharacter, BaseHealth);
    DOREPLIFETIME(AEnemyCharacter, bIsDead);
    DOREPLIFETIME(AEnemyCharacter, CharacterTags);
    DOREPLIFETIME(AEnemyCharacter, MaxHealth);
    DOREPLIFETIME(AEnemyCharacter, PerceptionState);
    DOREPLIFETIME(AEnemyCharacter, LastKnownPlayerPos);
}


// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyCharacter::Attack()
{
    if (bIsDead || !AttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attack Montage not valid"));
        return;
    }

    // Route to server; AI calls usually originate on server, but this makes it robust.
    if (!HasAuthority())
    {
        Server_Attack();
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < NextAttackTime) return;// cooldown

    if (AActor* Target = GetBBTarget(Cast<AAIController>(GetController()), BBKey_Player))
    {
        if (FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) >
            FMath::Square(AttackRange)) return;
    }

    NextAttackTime = Now + AttackCooldown;

    const FName Section = FMath::RandBool() ? TEXT("StrikeLeft") : TEXT("StrikeRight");
    Multicast_PlayAttackMontage(Section);
}

void AEnemyCharacter::Server_Attack_Implementation()
{
    Attack(); // reuse server-side validation + multicast.
}

void AEnemyCharacter::Multicast_PlayAttackMontage_Implementation(FName SectionName)
{
    if (!AttackMontage || !GetMesh()) return;

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        // avoid double-plays if rapid multicast happens
        if (!Anim->Montage_IsPlaying(AttackMontage))
        {
            Anim->Montage_Play(AttackMontage, 1.0f);
        }

        if (SectionName != NAME_None)
        {
            Anim->Montage_JumpToSection(SectionName, AttackMontage); // same section for everyone
        }
    }
}

float AEnemyCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || bIsDead) return 0.f;

    const APawn* Killer = DamageHelpers::ResolveKillerPawn(EventInstigator, DamageCauser);

    if (DamageHelpers::IsZombiesMode(GetWorld()))
    {
        if (!Killer || !Killer->IsPlayerControlled()) return 0.f; // only players hurt zombies
        if (DamageHelpers::IsZombieActor(Killer)) return 0.f; // block zombie→zombie
    }

    BaseHealth = FMath::Clamp(BaseHealth - DamageAmount, 0.f, MaxHealth);
    if (BaseHealth <= 0.f && !bIsDead)
    {
        bIsDead = true;
        ApplyDeadState(); 
        ForceNetUpdate(); 
        GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AEnemyCharacter::Die, 3.0f, false);
    }
    return DamageAmount;
}

void AEnemyCharacter::Die()
{
    if (HasAuthority()) 
    {
        if (OnZombieDeath.IsBound())
        {
            OnZombieDeath.Broadcast();
        }

        Destroy();
    }
}


void AEnemyCharacter::OnRep_IsDead()
{
    if (bIsDead) ApplyDeadState();
}

void AEnemyCharacter::ApplyCharacterStats()
{
    // Clean, consistent application of character stats
    // apply character stats for speed and so on
    if (!HasAuthority()) return;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed *= CharacterStats.SpeedMultiplier;
    }
    MaxHealth = FMath::Max(1.f, MaxHealth * CharacterStats.HealthMultiplier);
    AttackDamage *= CharacterStats.DamageMultiplier;
    BaseHealth = MaxHealth; // reset current to new max once
}

void AEnemyCharacter::SetPerceptionState(bool bSee, bool bHear, bool bInRange)
{
    if (!HasAuthority()) return;

    const bool bChanged =
        (PerceptionState.bSee != bSee) ||
        (PerceptionState.bHear != bHear) ||
        (PerceptionState.bInRange != bInRange);

    if (!bChanged) return;

    PerceptionState.bSee = bSee;
    PerceptionState.bHear = bHear;
    PerceptionState.bInRange = bInRange;

    SetNetDormancy(DORM_Awake);
    FlushNetDormancy();
    ForceNetUpdate();
}

void AEnemyCharacter::SetChasingState(bool bChasing)
{
    if (!HasAuthority()) return;

    if (PerceptionState.bChasing == bChasing) return;    

        PerceptionState.bChasing = bChasing;
    ForceNetUpdate();
}


void AEnemyCharacter::OnRep_PerceptionState()
{
    if (UAI_AnimInstance* AI = Cast<UAI_AnimInstance>(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
    {
        AI->SetPlayerVisibility(PerceptionState.bSee);
        AI->SetPlayerAttackRange(PerceptionState.bInRange);
    }
}
void AEnemyCharacter::ApplyDeadState()
{
    if (HasAuthority())
    {
        if (AAIController* AIC = Cast<AAIController>(GetController()))
        {
            AIC->StopMovement();
            if (UBrainComponent* Brain = AIC->GetBrainComponent())
            {
                Brain->StopLogic(TEXT("Dead"));
            }
        }

        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            Move->DisableMovement();
        }
    }

    if (UAI_AnimInstance* AI = Cast<UAI_AnimInstance>(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
    {
        AI->SetIsDead(true);
    }
}


void AEnemyCharacter::SetLastKnownPlayerPos(const FVector& InPos)
{
    if (!HasAuthority()) return;

    const FVector_NetQuantize10 Q = InPos;
    if (LastKnownPlayerPos.Equals(Q, 1.f)) return; // avoid spam

    LastKnownPlayerPos = Q;

    SetNetDormancy(DORM_Awake);
    FlushNetDormancy();
    ForceNetUpdate();
}

void AEnemyCharacter::OnRep_LastKnownPlayerPos()
{
    // Example: drive anim/UI direction on clients
    if (UAI_AnimInstance* AI = Cast<UAI_AnimInstance>(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
    {
        const FVector Dir2D = (FVector(LastKnownPlayerPos) - GetActorLocation()).GetSafeNormal2D();
        // AI->SetLastKnownDirYaw(Dir2D.Rotation().Yaw);  // add if you want
    }
}



