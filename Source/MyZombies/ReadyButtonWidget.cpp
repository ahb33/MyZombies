// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyButtonWidget.h"
#include "Components/Button.h"
#include "MyPlayerController.h"

void UReadyButtonWidget::NativeConstruct()
{
    if (ReadyButton)
    {
        ReadyButton->OnClicked.AddDynamic(this, &UReadyButtonWidget::OnClicked);
    }
}

void UReadyButtonWidget::OnClicked()
{
    if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer()))
    {
        PC->HandleReadyInput();
    }
}