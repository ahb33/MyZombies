// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMenuWidget.h"
#include "MenuUIManager.h"
#include "MyPlayerController.h"

// Fill out your copyright notice in the Description page of Project Settings.



void UBaseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GetWorld() || GetWorld()->IsNetMode(NM_DedicatedServer) || !GetOwningPlayer())
	{
		return;
	}

	CachedPC = Cast<AMyPlayerController>(GetOwningPlayer());
	OnMenuShown();
}

void UBaseMenuWidget::NativeDestruct()
{
	OnMenuHidden();
	Super::NativeDestruct();
}

AMyPlayerController* UBaseMenuWidget::GetMyPC() const
{
	if (CachedPC.IsValid())
	{
		return CachedPC.Get();
	}
	return Cast<AMyPlayerController>(GetOwningPlayer());
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