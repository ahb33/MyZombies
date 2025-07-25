// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMenuWidget.h"
#include "MyPlayerController.h"

// Fill out your copyright notice in the Description page of Project Settings.



void UBaseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();


    // Skip only on *dedicated* servers
    if (!GetWorld() || GetWorld()->IsNetMode(NM_DedicatedServer) || !GetOwningPlayer())
    {
        return;
    }

    if (APlayerController* PC = GetOwningPlayer())
    {
        playerController = Cast<AMyPlayerController>(PC);
        if (playerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("BaseMenuWidget: Successfully cast and stored player controller."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("BaseMenuWidget: Failed to cast to AMyPlayerController."));
        }
    }

    if (!playerController)
    {
        UE_LOG(LogTemp, Error, TEXT("UBaseMenuWidget::NativeConstruct: Failed to get player controller!"));
        return;
    }
}



void UBaseMenuWidget::TransitionToMenu(FName MenuName)
{
    if (IsRunningDedicatedServer()) return;  // Skip on server

    // Check if the widget is already created and stored
    if (UUserWidget** FoundWidget = menuWidgetMap.Find(MenuName))
    {
        if (*FoundWidget)
        {
            // Remove the current widget and display the new one
            RemoveFromParent();  // Ensure removal from the viewport
            (*FoundWidget)->AddToViewport();  // Add the found widget to the viewport
            return;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Widget for %s not found in map!"), *MenuName.ToString());
}



void UBaseMenuWidget::CreateAndStoreWidget(FName MenuName, TSubclassOf<UUserWidget> WidgetClass)
{
    if (IsRunningDedicatedServer()) return;  // Skip on server

    if (WidgetClass && !menuWidgetMap.Contains(MenuName))
    {
        UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
        if (NewWidget)
        {
            menuWidgetMap.Add(MenuName, NewWidget);
        }
    }
}

void UBaseMenuWidget::MenuSetup()
{
    if (IsRunningDedicatedServer()) return;  // Skip on server

    // Check if the widget is already in the viewport
    if (!IsInViewport())
    {
        AddToViewport();
        if(playerController)
        {
            SetupInputMode();  // Ensure input mode is set
            
        }
    }
    else
    {
        // UE_LOG(LogTemp, Warning, TEXT("BaseMenu is already in the viewport, skipping AddToViewport"));
    }
}

void UBaseMenuWidget::SetupInputMode()
{
    if (IsRunningDedicatedServer()) return;  // Skip on server

    if (!playerController)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupInputMode: playerController is NULL!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Player Controller valid - SetUpInputMode called"));
    FInputModeUIOnly InputModeData;
    InputModeData.SetWidgetToFocus(nullptr);
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    playerController->bShowMouseCursor = true;
    playerController->SetInputMode(InputModeData);
    UE_LOG(LogTemp, Warning, TEXT("SetupInputMode called - Mouse cursor visible."));
}


