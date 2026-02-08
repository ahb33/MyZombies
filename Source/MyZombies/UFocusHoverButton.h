// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UFocusHoverButton.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Focus Hover Button"))
class MYZOMBIES_API UUFocusHoverButton : public UButton
{
	GENERATED_BODY()
	
public:
	// /** If true, the button will render as Hovered while it has focus. */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	// bool bUseHoverStyleWhenFocused = true;

protected:
	// virtual void SynchronizeProperties() override;
	// virtual void OnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	// virtual void OnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
	// bool bDefaultCached = false;
	// FButtonStyle DefaultStyle;

	// void CacheDefaultStyleIfNeeded();
	// void ApplyFocusVisual(bool bFocused);
};