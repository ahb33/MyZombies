// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseMenuWidget.h"
#include "SoloMenuWidget.generated.h"



/**

 * 

*/

class UButton;

UCLASS()
class MYZOMBIES_API USoloMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

/*

	buttons for difficult set here
	access gamemode class that does so
*/
protected:
	virtual void NativeOnInitialized() override;

	virtual UWidget* GetDefaultFocusWidget_Implementation() const override;


private:
	UFUNCTION() void OnEasyButtonClicked();
	UFUNCTION() void OnMediumButtonClicked();
	UFUNCTION() void OnHardButtonClicked();
	UFUNCTION() void OnBackButtonClicked();

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true")) 
	TObjectPtr<UButton> EasyButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true")) 
	TObjectPtr<UButton> MediumButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true")) 
	TObjectPtr<UButton> HardButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="UI|Widgets", meta=(BindWidgetOptional, AllowPrivateAccess="true")) 
	TObjectPtr<UButton> BackButton = nullptr;
};