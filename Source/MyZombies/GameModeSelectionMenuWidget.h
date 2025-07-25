// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenuWidget.h"
#include "GameModeSelectionMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UGameModeSelectionMenuWidget : public UMainMenuWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    virtual void MenuSetup();

    void BindButtonEvents();

protected:
    UFUNCTION()
    void OnSelectDeathmatchButtonClicked();

    UFUNCTION()
    void OnSelectZombiesButtonClicked();

    UFUNCTION()
    void HandleGameModeSelection(FName GameModeName);

    UFUNCTION()
    void OnBackToMainMenuButtonClicked();


private:
    void LoadLobby(FString SelectedGameMode);

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* DeathmatchButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* ZombiesButton;
 
    UPROPERTY(meta = (BindWidgetOptional))
	class UButton* BackButton;


};
