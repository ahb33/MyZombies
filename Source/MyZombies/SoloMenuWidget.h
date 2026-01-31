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

private:
	UFUNCTION() void OnEasyButtonClicked();
	UFUNCTION() void OnMediumButtonClicked();
	UFUNCTION() void OnHardButtonClicked();
	UFUNCTION() void OnBackButtonClicked();

	UPROPERTY(meta=(BindWidgetOptional)) 
	TObjectPtr<UButton> EasyButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional)) 
	TObjectPtr<UButton> MediumButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional)) 
	TObjectPtr<UButton> HardButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional)) 
	TObjectPtr<UButton> BackButton = nullptr;
};