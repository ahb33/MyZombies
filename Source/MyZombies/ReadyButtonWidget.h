// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReadyButtonWidget.generated.h"

/**
 * 
 */

class UButton;

UCLASS()
class MYZOMBIES_API UReadyButtonWidget : public UUserWidget
{
	GENERATED_BODY()
public:

    UFUNCTION(BlueprintCallable)
    void SetReadyPending(bool bPending);


protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
    TObjectPtr<UButton> ReadyButton = nullptr;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly)
    class UTextBlock* ButtonPrompt = nullptr;

    UFUNCTION()
    void OnClicked();
	
};
