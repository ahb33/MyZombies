// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "MyPlayerController.h"


void UReadyButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);
    
    if (ReadyButton && !ReadyButton->OnClicked.IsAlreadyBound(this, &UReadyButtonWidget::OnClicked))
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

void UReadyButtonWidget::SetReadyPending(bool bPending)
{
    if (ReadyButton) 
    ReadyButton->SetIsEnabled(!bPending);
    
    if (ButtonPrompt)  
    ButtonPrompt->SetText(bPending ? FText::FromString(TEXT("...waiting for players to join...")) : FText::FromString(TEXT("Press Enter")));
}