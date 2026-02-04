// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

/**
 * 
 */

class UButton;
class AMyPlayerController;


UCLASS()
class MYZOMBIES_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	
public:

    virtual void NativeOnInitialized() override;

	UFUNCTION()
	void ConfigureForMultiplayer(bool bIsMultiplayer);
		

protected:

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
	TObjectPtr<UButton> StartOverButton = nullptr;

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
	TObjectPtr<UButton> QuitButton = nullptr;

	UPROPERTY(meta = (BindWidget),  BlueprintReadOnly)
	TObjectPtr<UButton> ResumeButton = nullptr;

	UFUNCTION()
    void OnStartOverButtonClicked();

	UFUNCTION()
    void OnResumeButtonClicked();

	UFUNCTION()
    void OnQuitButtonClicked();

	AMyPlayerController* GetMyPC() const;
};


