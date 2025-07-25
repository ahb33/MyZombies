// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModeSelectionMenuWidget.h"
#include "MultiplayerSessions.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerMenuWidget.generated.h"

/**
 * 
 */


// this class should be child of gamemode selection and gamemode selection shou
UCLASS()
class MYZOMBIES_API UMultiplayerMenuWidget : public UGameModeSelectionMenuWidget
{
	GENERATED_BODY()

public:

    virtual void NativeConstruct() override;

    virtual void MenuSetup();

    void BindButtonEvents();
  
    UFUNCTION()
    void OnBackButtonClicked();

	
private:
    UPROPERTY(meta = (BindWidget))
    UButton* MultiplayerBackButton;
};
