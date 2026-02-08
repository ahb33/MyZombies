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
    if (HasAuthority())
    {
        BaseHealth = MaxHealth;              
    }
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEnemyCharacter, BaseHealth);
    DOREPLIFETIME(AEnemyCharacter, bIsDead);
    DOREPLIFETIME(AEnemyCharacter, CharacterTags);


    DOREPLIFETIME(AEnemyCharacter, bCanSeePlayer);
    DOREPLIFETIME(AEnemyCharacter, bCanHearPlayer);
    DOREPLIFETIME(AEnemyCharacter, bPlayerWithinRange);
    DOREPLIFETIME(AEnemyCharacter, bIsChasing);
    DOREPLIFETIME(AEnemyCharacter, MaxHealth);
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
        ApplyDeadState();   // server visuals
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
        (bCanSeePlayer != bSee) ||
        (bCanHearPlayer != bHear) ||
        (bPlayerWithinRange != bInRange);

        bCanSeePlayer = bSee;
        bCanHearPlayer = bHear;
        bPlayerWithinRange = bInRange;

        if (bChanged) ForceNetUpdate();
}


void AEnemyCharacter::SetChasingState(bool bChasing)
{
    if (!HasAuthority()) return;
    if (bIsChasing == bChasing) return;
    bIsChasing = bChasing;
    ForceNetUpdate();
}

