// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YouDiedMenuWidget.generated.h"

/**
 * 
 */

class UButton;

UCLASS()
class MYZOMBIES_API UYouDiedMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
    UButton* PlayAgainButton;

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
    UButton* ExitToMainMenuButton;
  
	UFUNCTION()
	void OnPlayAgainClicked();

	UFUNCTION()
	void OnExitToMainMenuClicked();
	
	
};
