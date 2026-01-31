// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModeSelectionMenuWidget.h"
#include "MultiplayerSessions.h"
#include "MultiplayerMenuWidget.generated.h"

/**
 * 
 */


class UButton;

// this class should be child of gamemode selection and gamemode selection shou
UCLASS()
class MYZOMBIES_API UMultiplayerMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnBackButtonClicked();

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> MultiplayerBackButton = nullptr;
};