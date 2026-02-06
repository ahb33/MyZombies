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

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable)
    bool SetupMultiplayerBinding();

    UFUNCTION(BlueprintCallable)
    void AttemptFindSessions();

    UFUNCTION(BlueprintPure, Category="GameMode")
    FName GetSelectedGameModeCached() const;

    UFUNCTION(BlueprintPure)
    AMyPlayerController* GetMyPlayerController() {return CachedMyPlayerController;}

protected:

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> SessionsScrollBox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Multiplayer")
	TSubclassOf<UUserWidget> SessionItemClass;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SessionsMessage = nullptr;
    
private:

    bool EnsureCachedRefs();

private:
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Multiplayer", meta = (AllowPrivateAccess = "true"))
    class UMultiplayerSessions* MultiplayerSessionRef = nullptr;

    UPROPERTY()
    TObjectPtr<UMyGameInstance> CachedGameInstance = nullptr;

    UPROPERTY(Transient, BlueprintReadWrite, Category="Multiplayer", meta=(AllowPrivateAccess="true"))
	TArray<FMySessionResult> CachedSessionResults;

	UPROPERTY(Transient)
	TObjectPtr<AMyPlayerController> CachedMyPlayerController = nullptr;

};