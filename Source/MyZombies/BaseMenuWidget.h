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

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category="UI|Focus")
	UWidget* GetDefaultFocusWidget() const;

	virtual UWidget* GetDefaultFocusWidget_Implementation() const { return DefaultFocusWidget; }

	UFUNCTION(BlueprintCallable, Category="UI|Focus")
    void ApplyInitialFocus();


protected:
	virtual void NativeOnInitialized() override;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI|Focus") 
	TObjectPtr<UWidget> DefaultFocusWidget = nullptr;

private:
	mutable TWeakObjectPtr<AMyPlayerController> CachedPC;
};