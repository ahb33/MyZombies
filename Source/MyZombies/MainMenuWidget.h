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
	
 	virtual UWidget* GetDefaultFocusWidget_Implementation() const override;

private:
	UFUNCTION()
	void OnSoloClicked();

	UFUNCTION()
	void OnMultiplayerClicked();

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true"))	
	TObjectPtr<UButton> SoloButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true"))
	TObjectPtr<UButton> MultiplayerButton = nullptr;


};
