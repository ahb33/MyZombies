// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.h"
#include "MyPlayerController.h"
#include "SoloMenuWidget.generated.h"



/**

 * 

*/
UCLASS()
class MYZOMBIES_API USoloMenuWidget : public UMainMenuWidget
{
	GENERATED_BODY()

/*

	buttons for difficult set here
	access gamemode class that does so
*/
protected:
    virtual void NativeConstruct() override;

public:

    virtual void MenuSetup();
  
    void BindButtonEvents();

    UFUNCTION()
    void OnHardButtonClicked();

    UFUNCTION()
    void OnMediumButtonClicked();

    UFUNCTION()
    void OnEasyButtonClicked();

    UFUNCTION()
    void OnBackButtonClicked();


private:

    UPROPERTY(meta = (BindWidget))
	class UButton* EasyButton;

    UPROPERTY(meta = (BindWidget))
	class UButton* MediumButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HardButton;

    UPROPERTY(meta = (BindWidget))
	class UButton* BackButton;
	
};
