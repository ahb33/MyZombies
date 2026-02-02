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


    if (ServerNameLabel)
    {
        ServerNameLabel->SetText(FText::FromString(InSession.OwningUserName));
    }
    
    if (NumOfPlayersLabel)
    {
        NumOfPlayersLabel->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), 
        InSession.CurrentPlayers, InSession.MaxPlayers)));
    }

	if (Ping)
	{
		Ping->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), InSession.PingInMs)));
	}

	if (ModeLabel)
	{
		ModeLabel->SetText(FText::FromString(InSession.MatchType));
	}
}


void UJoinSessionMenuWidget::HandleJoinFinished(bool bSuccess)
{
	OnBusyChanged.Broadcast(false);
}

void UJoinSessionMenuWidget::OnJoinSessionClicked()
{
    OnBusyChanged.Broadcast(true);

    if (const UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
    {
        if (UMultiplayerSessions* Sessions = GI->GetMultiplayerSessions())
        {
            Sessions->JoinSessionByIndex(SessionIndex);
        }
    }

    DisableJoinSessionButton();

}

void UJoinSessionMenuWidget::DisableJoinSessionButton()
{
    if (JoinSessionLabel)
    {
        JoinSessionLabel->SetText(FText::FromString(TEXT("Joining Session")));
    }
    JoinButton->SetIsEnabled(false);

}