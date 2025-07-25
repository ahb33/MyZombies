#include "JoinSessionMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MyGameInstance.h"

void UJoinSessionMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (JoinButton && !JoinButton->OnClicked.IsAlreadyBound(this, &UJoinSessionMenuWidget::OnJoinSessionClicked))
    {
        JoinButton->OnClicked.AddDynamic(this, &UJoinSessionMenuWidget::OnJoinSessionClicked);
    }
}

void UJoinSessionMenuWidget::Setup(int32 InIndex, const FMySessionResult& InSession)
{
    SessionIndex = InIndex;
    ServerName = InSession.OwningUserName;
    PlayerCountText = FString::Printf(TEXT("%d/%d"), InSession.CurrentPlayers, InSession.MaxPlayers);

    if (ServerNameLabel)
    {
        UE_LOG(LogTemp, Warning, TEXT("ServerNameLabel is valid"));
        ServerNameLabel->SetText(FText::FromString(ServerName));
    }
    
    if (NumOfPlayersLabel)
    {
        UE_LOG(LogTemp, Warning, TEXT("NumOfPlayersLabel is valid"));
        NumOfPlayersLabel->SetText(FText::FromString(PlayerCountText));
    }
}


void UJoinSessionMenuWidget::OnJoinSessionClicked()
{
    if (const UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
    {
        if (UMultiplayerSessions* Sessions = GI->GetMultiplayerSessions())
        {
            Sessions->JoinSessionByIndex(SessionIndex);
        }
    }
}