// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "MyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Animation/AnimInstance.h"
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
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

    // (Optional) also ensure we're still in range before starting
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
    // Keep actual logic in Attack(); this call merely ensures server entry.
    Attack(); // why: reuses server-side validation + multicast.
}

void AEnemyCharacter::Multicast_PlayAttackMontage_Implementation(FName SectionName)
{
    if (!AttackMontage || !GetMesh()) return;

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        // why: avoid double-plays if rapid multicast happens
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
        GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AEnemyCharacter::Die, 1.0f, false);
    }
    return DamageAmount;
}



void AEnemyCharacter::Die()
{
    if (HasAuthority()) 
    {
        Destroy();
        if (OnZombieDeath.IsBound())
        {
            OnZombieDeath.Broadcast();
        }
    }
}


void AEnemyCharacter::OnRep_IsDead()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep is dead"));
    if (UAI_AnimInstance* AI = Cast<UAI_AnimInstance>(GetMesh()->GetAnimInstance()))
    {
        AI->SetIsDead(true); // ensures death anim plays on all clients
    }
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

