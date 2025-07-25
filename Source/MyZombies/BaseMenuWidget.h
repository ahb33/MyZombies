// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseMenuWidget.generated.h"

/**
 * 
 */

 class AMyPlayerController; 
 
UCLASS()
class MYZOMBIES_API UBaseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    // Create and store the widget
    void CreateAndStoreWidget(FName MenuName, TSubclassOf<UUserWidget> WidgetClass);
    AMyPlayerController* playerController;

public:

    virtual void TransitionToMenu(FName MenuName);
    virtual void MenuSetup();

    void SetupInputMode();

    // Const getter for child classes to access
    const TSubclassOf<UUserWidget>& GetMultiplayerMenuWidgetClass() const
    {
        return multiplayerMenuWidgetClass;
    }

    // Const getter for child classes to access
    const TSubclassOf<UUserWidget>& GetMainMenuWidgetClass() const
    {
        UE_LOG(LogTemp, Warning, TEXT("Get Main Menu Widget called"));
        return mainMenuWidgetClass;
    }

    // Const getter for child classes to access
    const TSubclassOf<UUserWidget>& GetSoloMenuWidgetClass() const
    {
        return SoloMenuWidgetClass;
    }

    // Const getter for child classes to access
    const TSubclassOf<UUserWidget>& GetGameModeSelectionMenuWidgetClass() const
    {
        return gameModeSelectionMenuWidgetClass;
    }

    // // Const getter for child classes to access
    // const TSubclassOf<UUserWidget>& GetLobbyMenuWidgetClass() const
    // {
    //     return lobbyMenuWidgetClass;
    // }

protected: 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widgets")
    TMap<FName, UUserWidget*> menuWidgetMap;  // A map that stores widget instances



private:


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> multiplayerMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> SoloMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> mainMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> gameModeSelectionMenuWidgetClass;

    


};
