// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "BaseMenuWidget.generated.h"

/**
 * 
 */

class AMyPlayerController;
class UMenuUIManager;
class UButton; 
 
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

	UFUNCTION(BlueprintCallable, Category="UI|Focus")
	void FocusWidget(UWidget* WidgetToFocus); // (optional) call from Button->OnHovered

protected:
    virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;


	UFUNCTION(BlueprintCallable)
	AMyPlayerController* GetMyPC() const;

	UMenuUIManager* GetMenuUI() const;


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
	TWeakObjectPtr<UWidget> LastFocusedWidget;

	TWeakObjectPtr<UButton> FocusStyledButton;
	FButtonStyle FocusStyledButtonOriginalStyle;
};