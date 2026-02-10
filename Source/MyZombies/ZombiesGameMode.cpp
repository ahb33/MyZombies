// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombiesGameMode.h"
#include "AI_EnemySpawner.h"
#include "BaseGameState.h"
#include "ZombiesGameState.h"
#include "Kismet/GameplayStatics.h"

AZombiesGameMode::AZombiesGameMode()
{
    CurrentLevel = 1; // Initialize with the first wave
    RemainingEnemies = 0; // Initialize enemy count
    GameStateClass = AZombiesGameState::StaticClass();
}

void AZombiesGameMode::BeginPlay()
{
    Super::BeginPlay();

	if (ABaseGameState* GS = GetGameState<ABaseGameState>())
	{
		GS->SetInputProfile(EInputProfile::Gameplay);
		GS->SetMatchMode(EMatchMode::Zombies);
	}
    
    ApplyLevelModifiers();
    StartNextWave();

}

void AZombiesGameMode::HandlePlayerDeath(AController* Victim, AController* Killer)
{
    SetMatchPhase(EMatchPhase::GameOver);

    GetWorldTimerManager().SetTimer(
        GameOverDelayHandle,
        FTimerDelegate::CreateWeakLambda(this, [this]
        {
            if (UWorld* W = GetWorld())
            {
                UGameplayStatics::SetGamePaused(W, true);
            }
        }),
        1.0f,
        false
    );
}


void AZombiesGameMode::OnZombieSpawned()
{
    ++ZombiesSpawnedThisWave;
    ++RemainingEnemies;
}

void AZombiesGameMode::OnZombieKilled()
{
    --RemainingEnemies;
    TryAdvanceWave();
}

void AZombiesGameMode::TryAdvanceWave()
{
    if (ZombiesSpawnedThisWave >= ZombiesToSpawnThisWave && RemainingEnemies <= 0) // all zombies dead = round over
    {
        SetMatchPhase(EMatchPhase::RoundOver);
        ++CurrentLevel;
        ApplyLevelModifiers();
        StartNextWave();
    }
}


void AZombiesGameMode::ApplyLevelModifiers()
{
    if (!HasAuthority()) return;
    UE_LOG(LogTemp, Warning, TEXT("Applying level modifiers"));
    if (AIDifficultyTable)
    {
        static const FString ContextString(TEXT("Difficulty Context"));
        FAICharacterStats* Stats = AIDifficultyTable->FindRow<FAICharacterStats>(FName(*FString::Printf(TEXT("Level%d"), CurrentLevel)), ContextString);
        if (Stats)
        {
            const FString Diff = GetWorld() ? FString(GetWorld()->URL.GetOption(TEXT("Difficulty="), TEXT("Medium"))) : TEXT("Medium");

            float CountMult = 1.0f; 
            if (Diff.Equals(TEXT("Easy"), ESearchCase::IgnoreCase))      CountMult = 0.75f;
            else if (Diff.Equals(TEXT("Hard"), ESearchCase::IgnoreCase)) CountMult = 1.25f;
            // Set the number of zombies to spawn for the current wave
            NumberOfZombiesForCurrentLevel = Stats->ZombiesPerLevel;
            UE_LOG(LogTemp, Warning, TEXT("ZombiesPerLevel retrieved from data table: %d"), Stats->ZombiesPerLevel);

            // Log the applied settings for debugging
            UE_LOG(LogTemp, Log, TEXT("Wave %d stats applied: Number of Zombies = %d"), CurrentLevel, NumberOfZombiesForCurrentLevel);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No stats found for wave %d in the data table."), CurrentLevel);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AI Difficulty Table is not assigned!"));
    }
}

void AZombiesGameMode::StartNextWave()
{
    if(!HasAuthority()) return;

    ZombiesToSpawnThisWave = NumberOfZombiesForCurrentLevel;
    ZombiesSpawnedThisWave = 0;
    RemainingEnemies = 0;

    if (AZombiesGameState* ZGS = GetGameState<AZombiesGameState>())
    {
        ZGS->SetRoundNumber(CurrentLevel); // ✅ advance replicated round number
    }

    SetMatchPhase(EMatchPhase::Intro);    
    
    // delay actual spawning
    GetWorldTimerManager().SetTimer(WaveIntroTimer, this, &AZombiesGameMode::BeginWaveActive, WaveIntroDelay, false);
}

void AZombiesGameMode::BeginWaveActive()
{
    if(!HasAuthority()) return;

    SetMatchPhase(EMatchPhase::Active);

    // Retrieve all spawners in the game
    TArray<AActor*> SpawnerActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAI_EnemySpawner::StaticClass(), SpawnerActors);

    if (SpawnerActors.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No spawners found in the level!"));
        return;
    }

    TArray<AAI_EnemySpawner*> Spawners;
    Spawners.Reserve(SpawnerActors.Num());

    for (AActor* Actor : SpawnerActors)
    {
        if (AAI_EnemySpawner* S = Cast<AAI_EnemySpawner>(Actor))
        {
            Spawners.Add(S);
        }
    }

    if (Spawners.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Spawner actors found, but none cast to AAI_EnemySpawner!"));
        return;
    }

    int32 ZombiesToSpawn = NumberOfZombiesForCurrentLevel;
    TMap<AAI_EnemySpawner*, int32> SpawnPlan;

    while (ZombiesToSpawn > 0)
    {
        // Pick a random spawner
        const int32 RandomIndex = FMath::RandRange(0, Spawners.Num() - 1);
        if (AAI_EnemySpawner* EnemySpawner = Cast<AAI_EnemySpawner>(Spawners[RandomIndex]))
        {
            // Calculate zombies for this spawner
            const int32 SpawnCount = FMath::Min(FMath::RandRange(1, ZombiesToSpawn), ZombiesToSpawn);

            SpawnPlan.FindOrAdd(EnemySpawner) += SpawnCount;
            // Deduct from remaining zombies
            ZombiesToSpawn -= SpawnCount;
        }
    }

    for (const TPair<AAI_EnemySpawner*, int32>& Pair : SpawnPlan)
    {
        if (!Pair.Key || Pair.Value <= 0) continue;
        Pair.Key->InitZombieArray(Pair.Value);
        Pair.Key->SpawnZombies(Pair.Value);
    }
    UE_LOG(LogTemp, Warning, TEXT("Wave %d started with %d zombies."), CurrentLevel, NumberOfZombiesForCurrentLevel);
}

