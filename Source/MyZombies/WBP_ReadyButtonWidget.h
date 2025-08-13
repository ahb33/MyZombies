// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WBP_ReadyButtonWidget.generated.h"

/**
 * 
 */

class UButton;

UCLASS()
class MYZOMBIES_API UWBP_ReadyButtonWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
    UButton* ReadyButton;

    UFUNCTION()
    void OnClicked();
	
};
