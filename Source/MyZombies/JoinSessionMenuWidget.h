// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessions.h"
#include "JoinSessionMenuWidget.generated.h"

/**
 * 
 */

class UButton;
class UTextBlock;


UCLASS()
class MYZOMBIES_API UJoinSessionMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
    void Setup(int32 InIndex, const FMySessionResult& InSession);

    void DisableJoinSessionButton();

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnJoinSessionClicked();

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* ServerNameLabel;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* NumOfPlayersLabel;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* JoinSessionLabel;

private:
    int32 SessionIndex;

    FString ServerName;
    FString PlayerCountText;
};
