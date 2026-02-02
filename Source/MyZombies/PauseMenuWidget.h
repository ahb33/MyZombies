// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

/**
 * 
 */

class UButton;
UCLASS()
class MYZOMBIES_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
	TObjectPtr<UButton> StartOver = nullptr;

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
	TObjectPtr<UButton> Quit = nullptr;

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
	TObjectPtr<UButton> Resume = nullptr;
};

