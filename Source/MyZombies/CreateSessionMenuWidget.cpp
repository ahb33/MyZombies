#include "CreateSessionMenuWidget.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "MultiplayerSessions.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h"

void UCreateSessionMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CreateSessionButton)
    {
        CreateSessionButton->OnClicked.AddDynamic(this, &UCreateSessionMenuWidget::OnCreateSessionClicked);
    }

    if (const UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
    {
        MultiplayerSubsystem = GI->GetMultiplayerSessions();

        if (MultiplayerSubsystem && 
            !MultiplayerSubsystem->MultiplayerOnCreateSessionComplete.IsAlreadyBound(this, &UCreateSessionMenuWidget::OnCreateSessionComplete))
        {
            MultiplayerSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UCreateSessionMenuWidget::OnCreateSessionComplete);
        }
    }
}

void UCreateSessionMenuWidget::OnCreateSessionClicked()
{
    if (!MultiplayerSubsystem) return;

    int32 NumPlayers = GetSelectedPlayerCount();
    if (NumPlayers < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid player count selected."));
        return;
    }
    
    FString GameType = GetSelectedGameType();
    MultiplayerSubsystem->CreateSession(NumPlayers, GameType);
}

void UCreateSessionMenuWidget::OnCreateSessionComplete(bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("CreateSession finished. Success: %d"), bWasSuccessful);
    if (bWasSuccessful)
    {
        const int32 Desired = GetSelectedPlayerCount();
        const FString Options = FString::Printf(TEXT("listen?desired=%d"), Desired);
        UGameplayStatics::OpenLevel(this, TEXT("/Game/GameAssets/Levels/LobbyLevel"), true, Options);
    }
}

int32 UCreateSessionMenuWidget::GetSelectedPlayerCount() const
{
    if (NumOfPlayersBox)
    {
        FString Selected = NumOfPlayersBox->GetSelectedOption();
        return FCString::Atoi(*Selected);
    }
    return 0;
}

FString UCreateSessionMenuWidget::GetSelectedGameType() const
{
    if (const UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
    {
        return GI->GetSelectedGameMode().ToString();
    }
    return TEXT("Default");
}