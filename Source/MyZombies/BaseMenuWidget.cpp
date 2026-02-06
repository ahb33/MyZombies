// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMenuWidget.h"
#include "MenuUIManager.h"
#include "MyPlayerController.h"



// Fill out your copyright notice in the Description page of Project Settings.
void UBaseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(true); 
}

void UBaseMenuWidget::ApplyInitialFocus()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, PC]()
        {
            if (UWidget* Target = GetDefaultFocusWidget())
            {
                Target->SetUserFocus(PC);
                Target->SetKeyboardFocus();
            }
            else
            {
                SetUserFocus(PC);
                SetKeyboardFocus();
            }
        }));
    }
}

void UBaseMenuWidget::NativeDestruct()
{
	OnMenuHidden();
	Super::NativeDestruct();
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
