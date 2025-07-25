// Copyright Epic Games, Inc. All Rights Reserved.

#include "BaseGameMode.h"
#include "MyZombiesCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyCharacter.h"
#include "MyPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ABaseGameMode::ABaseGameMode() {bUseSeamlessTravel = true;}

// try to alter code so that begin play isnt called while in menu
void ABaseGameMode::BeginPlay()
{
    Super::BeginPlay();
    InitializeGameplay();

    // Ensure game is not paused
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    if (PlayerController)
    {
        PlayerController->SetPause(false);
        // PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->bShowMouseCursor = true;
        
    }    
}

void ABaseGameMode::InitializeGameplay()
{
    UE_LOG(LogTemp, Warning, TEXT("Base gameplay initialized."));
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