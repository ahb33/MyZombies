#include "FindSessionMenuWidget.h"
#include "MyGameInstance.h"
#include "JoinSessionMenuWidget.h"
#include "MyPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

void UFindSessionMenuWidget::SetupMultiplayerBinding()

{
    CachedGameInstance = Cast<UMyGameInstance>(GetGameInstance());
    if (!CachedGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UMyGameInstance."));
        return;
    }

    MultiplayerSessionRef = CachedGameInstance->GetMultiplayerSessions();
    if (!MultiplayerSessionRef)
    {
        UE_LOG(LogTemp, Error, TEXT("MultiplayerSessionRef is null — cannot proceed."));
        return;
    }
}

void UFindSessionMenuWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UFindSessionMenuWidget::AttemptFindSessions()
{
    if (!MultiplayerSessionRef)
    {
        UE_LOG(LogTemp, Error, TEXT("MultiplayerSessionRef is null — did you forget to call InitializeFindSessionFlow()?"));
        return;
    }

    AMyPlayerController* MyPC = GetMyPlayerController();
    if (!MyPC)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid player controller found."));
        return;
    }

    const FString MatchType = GetCurrentMatchType();
    UE_LOG(LogTemp, Warning, TEXT("Attempting FindSessions with MatchType: %s"), *MatchType);
    MultiplayerSessionRef->FindSessions(MyPC, 10, MatchType);
}

FString UFindSessionMenuWidget::GetCurrentMatchType() const
{
    return CachedGameInstance ? CachedGameInstance->GetSelectedGameMode().ToString() : TEXT("None");
}

AMyPlayerController* UFindSessionMenuWidget::GetMyPlayerController()
{
    if (!CachedMyPlayerController)
    {
        CachedMyPlayerController = Cast<AMyPlayerController>(GetOwningPlayer());
        if (!CachedMyPlayerController)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast PlayerController to AMyPlayerController"));
        }
    }
    return CachedMyPlayerController;
}
