// Source/MyZombies/MyGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MultiplayerSessions.h"
#include "MyGameInstance.generated.h"

class UUserWidget;
class UZombiesRoundWidget;
class USoundBase;
class UPauseMenuWidget;
class UGameOverMenuWidget;

UCLASS()
class MYZOMBIES_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="GameMode")
    void SetSelectedGameMode(FName GameMode) { SelectedGameMode = GameMode; }

    UFUNCTION(BlueprintCallable, Category="GameMode")
    FName GetSelectedGameMode() const { return SelectedGameMode; }

    UFUNCTION(Category="Multiplayer")
    UMultiplayerSessions* GetMultiplayerSessions() const
    {
        return GetSubsystem<UMultiplayerSessions>();
    }

    UFUNCTION(Category="UI|Menus")
    TSubclassOf<UUserWidget> GetMainMenuWidgetClass() const { return MainMenuWidgetClass; }

    UFUNCTION(Category="UI|Menus")
    TSubclassOf<UUserWidget> GetSoloMenuWidgetClass() const { return SoloMenuWidgetClass; }

    UFUNCTION(Category="UI|Menus")
    TSubclassOf<UUserWidget> GetGameModeSelectionMenuWidgetClass() const { return GameModeSelectionMenuWidgetClass; }

    UFUNCTION(Category="UI|Menus")
    TSubclassOf<UUserWidget> GetCreateSessionMenuWidgetClass() const { return CreateSessionMenuWidgetClass; }

    UFUNCTION(Category="UI|Menus")
    TSubclassOf<UUserWidget> GetJoinSessionMenuWidgetClass() const { return JoinSessionMenuWidgetClass; }

    UFUNCTION(Category="UI|Menus")
    TSubclassOf<UUserWidget> GetMultiplayerMenuWidgetClass() const { return MultiplayerMenuWidgetClass; }

    UFUNCTION(Category="UI|Round")
    TSubclassOf<UZombiesRoundWidget> GetRoundSplashWidgetClass() const { return RoundSplashWidgetClass; }

    UFUNCTION(Category="UI|Round")
    TSubclassOf<UZombiesRoundWidget> GetRoundHUDWidgetClass() const { return RoundHUDWidgetClass; }

    UFUNCTION(BlueprintPure, Category="UI|Menus")
    TSubclassOf<UPauseMenuWidget> GetPauseMenuWidgetClass() const { return PauseMenuWidgetClass; }

    UFUNCTION(BlueprintPure, Category="UI|Death")
    TSubclassOf<UGameOverMenuWidget> GetDeathScreenWidgetClass() const { return DeathScreenWidgetClass; }

    UFUNCTION(Category="Audio|Round")
    USoundBase* GetRoundThudSound() const { return RoundThudSound; }

    UFUNCTION(Category="UI|Round")
    float GetRoundIntroWidgetDuration() const { return RoundIntroWidgetDuration; }

private:
    UPROPERTY()
    FName SelectedGameMode;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> SoloMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> GameModeSelectionMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> CreateSessionMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> JoinSessionMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> MultiplayerMenuWidgetClass;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Round", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UZombiesRoundWidget> RoundSplashWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Menus", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Death", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UGameOverMenuWidget> DeathScreenWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Round", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UZombiesRoundWidget> RoundHUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio|Round", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USoundBase> RoundThudSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Audio|Round", meta=(AllowPrivateAccess="true"))
    TArray<TObjectPtr<USoundBase>> RoundVoiceSounds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Round", meta=(AllowPrivateAccess="true"))
    float RoundIntroWidgetDuration = 2.0f;
};