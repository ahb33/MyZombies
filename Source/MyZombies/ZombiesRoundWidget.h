// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZombiesRoundWidget.generated.h"

/**
 * 
 */

class UTextBlock;

UCLASS()
class MYZOMBIES_API UZombiesRoundWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    virtual void NativeConstruct() override;

	void SetRound(int32 Round);

private:

	UPROPERTY(meta=(BindWidget))
	UTextBlock* RoundText = nullptr;

};
