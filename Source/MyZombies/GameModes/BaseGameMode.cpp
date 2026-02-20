// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseGameMode.h"
#include "MyZombiesCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EnemyCharacter.h"
#include "MyPlayerController.h"
#include "UObject/ConstructorHelpers.h"


ABaseGameMode::ABaseGameMode() {bUseSeamlessTravel = true;}


void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Found);

	SpawnPoints.Reserve(Found.Num());
	for (AActor* Actor : Found)
	{
		if (APlayerStart* PS = Cast<APlayerStart>(Actor))
		{
			SpawnPoints.Add(PS);
		}
	}
}
/*

Runs on the server after a player logs in.  
Provides the player's Controller so you can spawn/possess their pawn and handle initialization.  

*/
void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		RestartPlayer(NewPlayer);
	}
}


// Selects a random spawn point from the available list, marks it as used, and returns it.
AActor* ABaseGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	while(SpawnPoints.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
		AActor* Chosen = SpawnPoints[Index];

		// Mark as used so it can't be chosen again
		SpawnPoints.RemoveAtSwap(Index, 1, false);

		if(IsValid(Chosen))
		{
			return Chosen;
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

ABaseGameState* ABaseGameMode::GetBaseGameState() const
{
	return GetGameState<ABaseGameState>();
}

void ABaseGameMode::SetMatchPhase(EMatchPhase NewPhase)
{
	if (ABaseGameState* GS = GetBaseGameState())
	{
		GS->SetMatchPhase(NewPhase);
	}
}

void ABaseGameMode::DestroyCurrentPawn(AController* Controller)
{
	if (!Controller) return;

	if (APawn* Pawn = Controller->GetPawn())
	{
		Pawn->Destroy();
	}
}

 
void ABaseGameMode::RestartPlayerSafe(AController* Controller)
{
	if (!Controller) return;
	RestartPlayer(Controller);
}

void ABaseGameMode::RequestSpawn(AController* Controller)
{
	if (!Controller) return;

	DestroyCurrentPawn(Controller);

	if (RespawnDelay <= 0.f)
	{
		RestartPlayerSafe(Controller);
		return;
	}

	FTimerHandle Handle;
	TWeakObjectPtr<AController> WeakController = Controller;
	GetWorldTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda([this, WeakController]()
		{
			if (WeakController.IsValid())
			{
				RestartPlayerSafe(WeakController.Get());
			}
		}),
		RespawnDelay,
		false
	);
}