// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_EnemySpawner.h"
#include "ZombiesGameMode.h"
#include "Components/BoxComponent.h"


// to fix spawning issues, we might have to tell spawner to generate pickups only if they have the authority to do so

// Sets default values
AAI_EnemySpawner::AAI_EnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
    RootComponent = SpawnArea;
}

// Called when the game starts or when spawned
void AAI_EnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	// BP_EnemyCharacterClass = LoadClass<AEnemyCharacter>(nullptr, TEXT("/Game/GameAssets/AI/Blueprints/BP_EnemyCharacter.BP_EnemyCharacter_C"));
    if (!BP_EnemyCharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load BP_EnemyCharacter class!"));
    }
}

void AAI_EnemySpawner::InitZombieArray(int32 NumberOfZombies)
{
    UE_LOG(LogTemp, Warning, TEXT("Number of Zombies is %d"), NumberOfZombies);
    UE_LOG(LogTemp, Warning, TEXT("InitZombiesArray Called"));

    ZombieSpawnArray.Empty(); // emptires array

    if (BP_EnemyCharacterClass)
    {
        ZombieSpawnArray.AddUnique(BP_EnemyCharacterClass); // Prevent duplicates
        TotalZombiesToSpawn = NumberOfZombies; // Set spawn limit per level
        ZombieSpawnCount = 0; // Reset spawn count
    }
}

void AAI_EnemySpawner::SpawnZombies(int32 NumberOfZombies)
{
	// if array is empty return
	// if actor does not have authority to spawn return
    if (ZombieSpawnArray.IsEmpty() || !HasAuthority()) return;
    
    // Spawning zombies with a cooldown interval
    GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AAI_EnemySpawner::Spawn, SpawnCooldown, true);
}

void AAI_EnemySpawner::Spawn()
{
    if (ZombieSpawnCount >= TotalZombiesToSpawn)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    const auto ZombieClass = ZombieSpawnArray[FMath::RandRange(0, ZombieSpawnArray.Num() - 1)];
    const auto SpawnRotation = FRotator::ZeroRotator;
    const auto SpawnLocation = SpawnArea->GetComponentLocation() + FVector(
        FMath::RandRange(-SpawnArea->GetScaledBoxExtent().X, SpawnArea->GetScaledBoxExtent().X),
        FMath::RandRange(-SpawnArea->GetScaledBoxExtent().Y, SpawnArea->GetScaledBoxExtent().Y),
        0.0f
    );

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

    if (AEnemyCharacter* SpawnedZombie = GetWorld()->SpawnActor<AEnemyCharacter>(ZombieClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        ZombieSpawnCount++;
        
        if (AZombiesGameMode* GameMode = GetWorld()->GetAuthGameMode<AZombiesGameMode>())
        {
            SpawnedZombie->OnZombieDeath.AddDynamic(GameMode, &AZombiesGameMode::OnZombieKilled);
            GameMode->OnZombieSpawned();
        }
    }
}

