// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverMenuWidget.generated.h"

/**
 * 
 */

class UButton;

UCLASS()
class MYZOMBIES_API UGameOverMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
   	TObjectPtr<UButton> PlayAgainButton;

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
    TObjectPtr<UButton> QuitButton;
  
	UFUNCTION()
	void OnPlayAgainClicked();

	UFUNCTION()
	void OnExitToMainMenuClicked();
	
	
};
