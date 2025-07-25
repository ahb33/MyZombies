// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessions.h"
#include "JoinSessionMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UJoinSessionMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
    void Setup(int32 InIndex, const FMySessionResult& InSession);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnJoinSessionClicked();

    UPROPERTY(meta = (BindWidget))
    class UButton* JoinButton;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    class UTextBlock* ServerNameLabel;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    class UTextBlock* NumOfPlayersLabel;

private:
    int32 SessionIndex;

    FString ServerName;
    FString PlayerCountText;
};
