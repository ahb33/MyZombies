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
	PrimaryActorTick.bCanEverTick = true;

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
    DOREPLIFETIME(AEnemyCharacter, BaseDamage);
    DOREPLIFETIME(AEnemyCharacter, bIsDead);
    DOREPLIFETIME(AEnemyCharacter, CharacterTags);
}

void AEnemyCharacter::OnBeginOverlap(AActor *OverlappedActor, AActor *OtherActor)
{
    // add code for when overlapping with character
    // debug line when overlapping 
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < NextAttackTime) return;// cooldown

    // (Optional) also ensure we're still in range before starting
    if (AActor* Target = GetBBTarget(Cast<AAIController>(GetController()), BBKey_Player))
    {
        if (FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) >
            FMath::Square(AttackRange)) return;
    }

    bHitAppliedThisSwing = false;
    NextAttackTime = Now + AttackCooldown;

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(AttackMontage);
        // randomize which strike plays
        static const FName Sections[] = { TEXT("StrikeLeft"), TEXT("StrikeRight") };
        Anim->Montage_JumpToSection(Sections[FMath::RandBool()], AttackMontage);
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
        bIsDead = true; OnRep_IsDead();
        GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AEnemyCharacter::Die, 1.0f, false);
    }
    OnRep_Health();
    return DamageAmount;
}


void AEnemyCharacter::AnimNotify_MeleeHit()
{
    // Single-frame contact (upgrade to AnimNotifyState for a window later)
    if (!HasAuthority() || bHitAppliedThisSwing || bIsDead) return;

    AAIController* AIC = Cast<AAIController>(GetController());
    AActor* Target = GetBBTarget(AIC, BBKey_Player);
    if (!Target) return;

    // Re-validate range at the contact frame
    const float DistSq2D = FVector::DistSquared2D(GetActorLocation(), Target->GetActorLocation());
    if (DistSq2D > FMath::Square(AttackRange)) return;

    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
    bHitAppliedThisSwing = true; // one hit per swing
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

void AEnemyCharacter::OnRep_Health()
{

}

void AEnemyCharacter::OnRep_IsDead()
{
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

