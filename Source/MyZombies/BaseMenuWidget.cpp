// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMenuWidget.h"
#include "MenuUIManager.h"
#include "MyPlayerController.h"
#include "Components/Button.h" 
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "InputCoreTypes.h"

// Fill out your copyright notice in the Description page of Project Settings.
void UBaseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);

	// make the root hit-testable so clicks on "empty space" don't fall through to the viewport.
	if (WidgetTree && WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UBaseMenuWidget::ApplyInitialFocus()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWidget* Target = GetDefaultFocusWidget();
			if (!IsValid(Target)) Target = this;
			FocusWidget(Target);
		}));
	}
}

void UBaseMenuWidget::FocusWidget(UWidget* WidgetToFocus)
{
	AMyPlayerController* PC = GetMyPC();
	if (!PC) return;

	// --- NEW: restore the previous focused button's original style
	if (FocusStyledButton.IsValid())
	{
		FocusStyledButton->SetStyle(FocusStyledButtonOriginalStyle);
		FocusStyledButton.Reset();
	}

	UWidget* Target = IsValid(WidgetToFocus) ? WidgetToFocus : GetDefaultFocusWidget();
	if (!IsValid(Target)) Target = this;

	LastFocusedWidget = Target;

	// Real focus
	Target->SetUserFocus(PC);
	Target->SetKeyboardFocus();

	if (UButton* Button = Cast<UButton>(Target))
	{
		FocusStyledButton = Button;
		FocusStyledButtonOriginalStyle = Button->WidgetStyle;

		FButtonStyle FocusStyle = FocusStyledButtonOriginalStyle;

		// Make "focused" look like "hovered"
		FocusStyle.SetNormal(FocusStyle.Hovered);
		FocusStyle.NormalForeground = FocusStyle.HoveredForeground; // helps text/icon tint match

		Button->SetStyle(FocusStyle);
	}
}

FReply UBaseMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Only fires when the click hits the menu root (i.e., empty space outside the frame).
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (UWidget* Target = LastFocusedWidget.Get())
		{
			FocusWidget(Target);
		}
		else
		{
			ApplyInitialFocus();
		}
		return FReply::Handled(); // prevents focus dumping to viewport
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

AMyPlayerController* UBaseMenuWidget::GetMyPC() const
{
	if (CachedPC.IsValid()) return CachedPC.Get();

	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer()))
	{
		CachedPC = PC;
		return PC;
	}
	return nullptr;
}

UMenuUIManager* UBaseMenuWidget::GetMenuUI() const
{
	if (AMyPlayerController* PC = GetMyPC())
	{
		return PC->GetMenuUI();
	}
	return nullptr;
}

void UBaseMenuWidget::RequestShowMenu(FName MenuId)
{
	if (UMenuUIManager* UI = GetMenuUI())
	{
		UI->ShowMenu(MenuId);
	}
}

void UBaseMenuWidget::RequestPushMenu(FName MenuId)
{
	if (UMenuUIManager* UI = GetMenuUI())
	{
		UI->PushMenu(MenuId);
	}
}

void UBaseMenuWidget::RequestPopMenu()
{
	if (UMenuUIManager* UI = GetMenuUI())
	{
		UI->PopMenu();
	}
}