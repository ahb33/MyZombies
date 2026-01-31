// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseMenuWidget.generated.h"

/**
 * 
 */

class AMyPlayerController;
class UMenuUIManager;

 
UCLASS()
class MYZOMBIES_API UBaseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	AMyPlayerController* GetMyPC() const;
	UMenuUIManager* GetMenuUI() const;

	// Optional hooks for child widgets
	virtual void OnMenuShown() {}
	virtual void OnMenuHidden() {}

	// Call these from buttons instead of TransitionToMenu/CreateAndStoreWidget
	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestShowMenu(FName MenuId);

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestPushMenu(FName MenuId);

	UFUNCTION(BlueprintCallable, Category="UI|Navigation")
	void RequestPopMenu();

private:
	TWeakObjectPtr<AMyPlayerController> CachedPC;
};