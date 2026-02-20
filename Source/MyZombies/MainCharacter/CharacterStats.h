// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterStats.generated.h"

/*
Need to add variables for progress bar and health text - you will access these variables in HUD class
Unreal Engine has  class UProgressBar*
*/

class UTextBlock;
class UProgressBar;

UCLASS()
class MYZOMBIES_API UCharacterStats : public UUserWidget
{
	GENERATED_BODY()

public: 

	void SetHealthBar(UProgressBar* NewHealthBar); // this function will be used to initialize HealthBar

	//This will allow you to select the class you need in the HUD when you create the widget
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoOnDisplay = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoInMag = nullptr;

	// add scores and kill count here for death match

};
