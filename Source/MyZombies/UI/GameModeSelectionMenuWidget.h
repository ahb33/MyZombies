// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenuWidget.h"
#include "BaseMenuWidget.h"
#include "GameModeSelectionMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UGameModeSelectionMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override; // called once per widget instance

	virtual UWidget* GetDefaultFocusWidget_Implementation() const override;


private:
	UFUNCTION()
	void OnSelectDeathmatchButtonClicked();

	UFUNCTION()
	void OnSelectZombiesButtonClicked();

    void BindButtonEvents();

	UFUNCTION()
	void OnBackButtonClicked();

	void HandleGameModeSelection(FName GameModeName);

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UButton> DeathmatchButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UButton> ZombiesButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UButton> BackButton = nullptr;
};
