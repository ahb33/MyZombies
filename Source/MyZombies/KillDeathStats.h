// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h" 
#include "KillDeathStats.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UKillDeathStats : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerKills;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerDeaths;
	
};
