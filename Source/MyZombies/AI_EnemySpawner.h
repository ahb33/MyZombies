// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyCharacter.h"
#include "AI_EnemySpawner.generated.h"

UCLASS()
class MYZOMBIES_API AAI_EnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAI_EnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	

	void InitZombieArray(int32 ); // this function will be used to initialze ZombieSpawnArray based on level we are in

	UFUNCTION()
	void SpawnZombies(int32 NumberOfZombies = 1);

	void Spawn();

private: 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Settings", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AEnemyCharacter> BP_EnemyCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Settings", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class AEnemyCharacter>> ZombieSpawnArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Settings", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> SpawnArea; 

	UPROPERTY()
	FTimerHandle SpawnTimerHandle; // interval used to spawn enemies
 
	int32 ZombieSpawnCount = 0;
    int32 TotalZombiesToSpawn;
    float SpawnCooldown = 1.5f;

};
