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
class UMultiplayerSessions;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBusyChangedBP, bool, bBusy);

UCLASS()
class MYZOMBIES_API UJoinSessionMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
    void Setup(int32 InIndex, const FMySessionResult& InSession);

    void DisableJoinSessionButton();

	UPROPERTY(BlueprintAssignable, Category="UI")
	FOnBusyChangedBP OnBusyChanged;

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnJoinSessionClicked();

	void HandleJoinFinished(bool bSuccess);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> JoinButton = nullptr;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> ServerNameLabel = nullptr;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> PingLabel = nullptr;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> ModeLabel = nullptr;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> NumOfPlayersLabel = nullptr;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextBlock> JoinSessionLabel = nullptr;

private:
    int32 SessionIndex;

    FString ServerName;
    FString PlayerCountText;
};
