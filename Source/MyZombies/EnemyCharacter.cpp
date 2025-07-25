// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "AI_EnemySpawner.h"
#include "Components/SphereComponent.h"
#include "BaseGameMode.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetSphereRadius(50);
    CollisionSphere->SetupAttachment(RootComponent);


}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Ensure AnimInstanceRef is of the type UAI_AnimInstance
    AnimInstanceRef = Cast<UAI_AnimInstance>(GetMesh()->GetAnimInstance());
    if (!AnimInstanceRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation instance is not of type UAI_AnimInstance!"));
    }
	
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEnemyCharacter, BaseHealth);
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


float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
AController* EventInstigator, AActor* DamageCauser)
{
    if (HasAuthority()) // Ensure damage is processed only on the server
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy %s took damage: %f"), *GetName(), DamageAmount);
        BaseHealth -= DamageAmount;
        UE_LOG(LogTemp, Warning, TEXT("Enemy BaseHealth is now: %f"), BaseHealth);

        if (BaseHealth <= 0.0f)
        {
            if (AnimInstanceRef)
            {
                AnimInstanceRef->SetIsDead(true);
            }
            GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AEnemyCharacter::Die, 1.0f, false);
        }
    }

    return DamageAmount; // Return the actual damage applied
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
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnZombieDeath not bound for %s"), *GetName());
        }
    }
}

void AEnemyCharacter::OnRep_Health()
{

}

void AEnemyCharacter::ApplyCharacterStats()
{
    // Clean, consistent application of character stats
    // apply character stats for speed and so on
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed *= CharacterStats.SpeedMultiplier;
    }
    BaseHealth *= CharacterStats.HealthMultiplier;
    BaseDamage *= CharacterStats.DamageMultiplier;

    UE_LOG(LogTemp, Log, TEXT("Character stats applied: Health = %f, Speed = %f, Damage = %f"), 
    BaseHealth, GetCharacterMovement()->MaxWalkSpeed, BaseDamage);
}

