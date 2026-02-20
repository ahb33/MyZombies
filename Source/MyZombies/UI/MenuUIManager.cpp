// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUIManager.h"
#include "BaseMenuWidget.h"
#include "MyPlayerController.h"
#include "UIHelpers.h"
#include "Blueprint/UserWidget.h"

void UMenuUIManager::Init(AMyPlayerController *InOwnerPC)
{
    OwnerPC = InOwnerPC;
    MenuStack.Reset();
    ActiveMenuID = NAME_None;
    ActiveMenu = nullptr;

}

void UMenuUIManager::RegisterMenu(FName MenuID, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!MenuID.IsNone() && WidgetClass)
	{
		MenuClasses.Add(MenuID, WidgetClass);
		MenuZOrder.Add(MenuID, ZOrder);
	}
}

UUserWidget* UMenuUIManager::GetOrCreateMenu(FName MenuID)
{
    if (!OwnerPC.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UMenuUIManager: OwnerPC is null."));
        return nullptr;
    }

    AMyPlayerController* PC = OwnerPC.Get();
    if (!IsValid(PC)) return nullptr;

    if (TObjectPtr<UUserWidget>* Found = MenuInstances.Find(MenuID))
    {
        if (IsValid(Found->Get()))
        {
            return Found->Get();
        }
        MenuInstances.Remove(MenuID);
    }


    TSubclassOf<UUserWidget>* ClassPtr = MenuClasses.Find(MenuID);
	if (!ClassPtr || !(*ClassPtr))
	{
		UE_LOG(LogTemp, Error, TEXT("UMenuUIManager: No class registered for MenuID '%s'."), *MenuID.ToString());
		return nullptr;
	}

    UUserWidget* Created = CreateWidget<UUserWidget>(PC, *ClassPtr);
	if (!Created)
	{
		UE_LOG(LogTemp, Error, TEXT("UMenuUIManager: CreateWidget failed for '%s'."), *MenuID.ToString());
		return nullptr;
	}

	MenuInstances.Add(MenuID, Created);
	return Created;
}


void UMenuUIManager::ShowMenu(FName MenuID)
{
	UUserWidget* Next = GetOrCreateMenu(MenuID);
	if (!Next) return;

	if (ActiveMenu && ActiveMenu != Next)
	{
		ActiveMenu->RemoveFromParent();
	}

	ActiveMenuID = MenuID;
	ActiveMenu = Next;

	const int32 ZOrder = MenuZOrder.Contains(MenuID) ? MenuZOrder[MenuID] : 0;
	if (!ActiveMenu->IsInViewport())
	{
		ActiveMenu->AddToViewport(ZOrder);
	}

    if (OwnerPC.IsValid())
    {
        AMyPlayerController* PC = OwnerPC.Get();
        if (IsValid(PC))
        {
            PC->ApplyInputProfile(EInputProfile::Menu, ActiveMenu.Get());
        }
    }

    if (UBaseMenuWidget* Base = Cast<UBaseMenuWidget>(ActiveMenu))
    {
        Base->ApplyInitialFocus(); // next tick: FocusWidget(default) sets input mode + keyboard focus correctly
    }
}


void UMenuUIManager::PushMenu(FName MenuID)
{
	if (!ActiveMenuID.IsNone())
	{
		MenuStack.Add(ActiveMenuID);
	}
	ShowMenu(MenuID);
}

void UMenuUIManager::PopMenu()
{
	if (MenuStack.Num() == 0)
	{
		if (ActiveMenu)
		{
			ActiveMenu->RemoveFromParent();
			ActiveMenu = nullptr;
			ActiveMenuID = NAME_None;
		}
		return;
	}

	const FName Prev = MenuStack.Pop();
	ShowMenu(Prev);
}




