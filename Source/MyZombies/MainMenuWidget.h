// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "BaseMenuWidget.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class MYZOMBIES_API UMainMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnSoloClicked();

	UFUNCTION()
	void OnMultiplayerClicked();

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SoloButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> MultiplayerButton = nullptr;
};
