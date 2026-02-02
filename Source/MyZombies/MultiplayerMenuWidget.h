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
class UBorder;
class UTextBlock;
class UPanelWidget;

// this class should be child of gamemode selection and gamemode selection shou
UCLASS()
class MYZOMBIES_API UMultiplayerMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetBusy(bool bBusy);

	UFUNCTION(BlueprintPure, Category="UI")
	bool IsBusy() const { return bIsBusy; }
	
protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnBackButtonClicked();


	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> MultiplayerBackButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBorder> BusyBlocker = nullptr;


	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> ContentRoot = nullptr;

	bool bIsBusy = false;

};