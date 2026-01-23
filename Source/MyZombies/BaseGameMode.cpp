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
#include "DamageQuery.h"

ABaseGameMode::ABaseGameMode() {bUseSeamlessTravel = true;}

// 
void ABaseGameMode::BeginPlay()
{
    Super::BeginPlay();

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AvailableSpawnPoints); // GetAllActorsOfClass Finds all actors of a class in the world and stores them in OutArray.


    
}

/*

Runs on the server after a player logs in.  
Provides the player's Controller so you can spawn/possess their pawn and handle initialization.  

*/
void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{

    Super::PostLogin(NewPlayer);

    if (AMyPlayerController* PC = Cast<AMyPlayerController>(NewPlayer))
    {
        // Spawn and possess the default pawn for this player
        RestartPlayer(PC);
    
    }
}

// Selects a random spawn point from the available list, marks it as used, and returns it.
AActor* ABaseGameMode::ChoosePlayerStart(AController* Player)
{
    
    if (AvailableSpawnPoints.Num() > 0)
    {

        int32 Index = FMath::RandRange(0, AvailableSpawnPoints.Num() - 1);
        AActor* Chosen = AvailableSpawnPoints[Index];
        
        UsedSpawnPoints.Add(Chosen);
        AvailableSpawnPoints.RemoveAt(Index);

        return Chosen;
    }

    return Super::ChoosePlayerStart(Player); // fallback if none left
}


 
void ABaseGameMode::EndGame(bool bPlayerWon)
{
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

    if (PlayerController)
    {
        PlayerController->SetPause(true);

        if (bPlayerWon && YouWonWidgetClass)
        {
            UUserWidget* YouWonWidget = CreateWidget<UUserWidget>(GetWorld(), YouWonWidgetClass);
            if (YouWonWidget)
            {
                YouWonWidget->AddToViewport();
                PlayerController->SetInputMode(FInputModeUIOnly());
                PlayerController->bShowMouseCursor = true;
            }
        }
    }
    else if (PlayerController)
    {
        PlayerController->SetPause(true);
    }
}

void ABaseGameMode::CheckEnemiesAlive()
{
    // Abstract method to be customized in derived classes
    UE_LOG(LogTemp, Warning, TEXT("Base CheckEnemiesAlive called."));
}

void ABaseGameMode::RequestSpawn(AController* Controller)
{
    if (!Controller) return;

    AMyPlayerController* PC = Cast<AMyPlayerController>(Controller);
    if (!PC) return;

    APawn* OldPawn = PC->GetPawn();
    if (OldPawn)
    {
        // TODO: Track which PlayerStart this pawn used if you want recycling
        OldPawn->Destroy();
    }

    RestartPlayer(PC);
}

bool ABaseGameMode::IsDamageAllowed(const AActor* Causer, const AActor* Target) const
{
    if (!Causer || !Target || Causer == Target) return true;

    const EUnitKind CK = UDamageQuery::GetUnitKind(Causer);
    const EUnitKind TK = UDamageQuery::GetUnitKind(Target);

    if (CK == EUnitKind::Player && TK == EUnitKind::Player && !bAllowPlayerVsPlayer) return false;
    if (CK == EUnitKind::AI     && TK == EUnitKind::AI     && !bAllowAIVsAI)         return false;

    const uint8 CT = UDamageQuery::GetTeamId(Causer);
    const uint8 TT = UDamageQuery::GetTeamId(Target);
    const bool SameTeam = (CT != 255 && TT != 255 && CT == TT);
    if (SameTeam && !bAllowFriendlyFireWithinTeam) return false;

    return true;
}


