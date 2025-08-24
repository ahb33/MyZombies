// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessions.h"
#include "FindSessionMenuWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UMultiplayerSessions;
class UMyGameInstance;
class AMyPlayerController;

// name
UCLASS()
class MYZOMBIES_API UFindSessionMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    void SetupMultiplayerBinding();

    UFUNCTION(BlueprintCallable)
    void AttemptFindSessions();

    FString GetCurrentMatchType() const;

    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintPure)
    AMyPlayerController* GetMyPlayerController();

    UPROPERTY(BlueprintReadWrite)
    TArray<FMySessionResult> CachedSessionResults;

protected:

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))

    
    UScrollBox* SessionsScrollBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
    TSubclassOf<UUserWidget> SessionItemClass;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* SessionsMessage;
    
private:
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Multiplayer", meta = (AllowPrivateAccess = "true"))
    class UMultiplayerSessions* MultiplayerSessionRef = nullptr;

    UPROPERTY()
    UMyGameInstance* CachedGameInstance = nullptr;

    UPROPERTY()
    AMyPlayerController* CachedMyPlayerController;

};