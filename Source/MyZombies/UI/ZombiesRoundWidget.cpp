// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombiesRoundWidget.h"
#include "Components/TextBlock.h"


void UZombiesRoundWidget::SetRound(int32 Round)
{    
    
    if (!RoundText) return;
    RoundText->SetText(FText::FromString(FString::Printf(TEXT("ROUND %d"), Round)));

}