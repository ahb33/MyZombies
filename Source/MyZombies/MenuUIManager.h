// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Blueprint/UserWidget.h"
#include "MenuUIManager.generated.h"

/**
 * Owns menu widget instances + transitions for ONE local player.
 * Why: keeps invariants (creation, focus/input, stack) out of individual menus.
 */

class AMyPlayerController;

UCLASS()
class MYZOMBIES_API UMenuUIManager : public UObject
{
	GENERATED_BODY()

public:

	void Init(AMyPlayerController* InOwnerPC);
	void RegisterMenu(FName MenuID, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0);

	UFUNCTION(BlueprintCallable)
	void ShowMenu(FName MenuID);

	UFUNCTION(BlueprintCallable)
	void PushMenu(FName MenuID);

	UFUNCTION(BlueprintCallable)
	void PopMenu();

	UFUNCTION(BlueprintCallable)
	FName GetActiveMenuID() const {return ActiveMenuID; }

	UFUNCTION(BlueprintPure, Category="UI|Navigation")
    UUserWidget* GetActiveMenuWidget() const { return ActiveMenu; }


private:
	UUserWidget* GetOrCreateMenu(FName MenuID);
private:

	UPROPERTY(Transient)
	TObjectPtr<AMyPlayerController> OwnerPC;

	UPROPERTY(Transient)
	TMap<FName, TSubclassOf<UUserWidget>> MenuClasses;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UUserWidget>> MenuInstances;

	UPROPERTY(Transient)
	TArray<FName> MenuStack;

	UPROPERTY(Transient)
	FName ActiveMenuID;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveMenu;

	UPROPERTY(Transient)
	TMap<FName, int32> MenuZOrder;
	
};
