
#include "MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "MyPlayerController.h"
#include "MultiplayerMenuWidget.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Only create the widget instance if it's not already in the array
    MenuSetup();

    
}


void UMainMenuWidget::MenuSetup()
{
    Super::MenuSetup();

    UE_LOG(LogTemp, Warning, TEXT("MainMenu constructor called"));
    
    BindButtonEvents();
}


// Ensure that buttons are valid and not already bound to prevent multiple bindings.
void UMainMenuWidget::BindButtonEvents()
{
    if (SoloButton && !SoloButton->OnClicked.IsAlreadyBound(this, &UMainMenuWidget::OnSoloClicked))
    {
        SoloButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSoloClicked);
    }

    if (MultiplayerButton && !MultiplayerButton->OnClicked.IsAlreadyBound(this, &UMainMenuWidget::OnMultiplayerClicked))
    {
        MultiplayerButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnMultiplayerClicked);
    }
}

void UMainMenuWidget::OnSoloClicked()
{
    // Ensure widget creation before attempting transition
    FName SoloMenu = "SoloMenu";

    auto soloMenuWidgetRef = GetSoloMenuWidgetClass();

    if (!menuWidgetMap.Contains(SoloMenu))
    {
        Super::CreateAndStoreWidget(SoloMenu, soloMenuWidgetRef);
    }

    TransitionToMenu(SoloMenu);
}

void UMainMenuWidget::OnMultiplayerClicked()
{
    // Ensure widget creation before attempting transition
    FName GameModeSelectionMenu = "GameModeSelectionMenu";

    auto gameModeSelectionMenuWidgetRef = GetGameModeSelectionMenuWidgetClass();

    if (!menuWidgetMap.Contains(GameModeSelectionMenu))
    {
        Super::CreateAndStoreWidget(GameModeSelectionMenu, gameModeSelectionMenuWidgetRef);
    }

    TransitionToMenu(GameModeSelectionMenu);
}



