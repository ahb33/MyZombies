#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "BaseGameState.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "PlayerUISubsystem.generated.h"

class AMyPlayerController;
class UMenuUIManager;
class UUserWidget;
class UAudioComponent;
class USoundBase;
class UGameOverMenuWidget;
class UZombiesRoundWidget;
class UPauseMenuWidget;

UENUM(BlueprintType)
enum class EOverlayInputMode : uint8
{
	None,
	UIOnly,
	GameAndUI
};

UCLASS(Blueprintable)
class MYZOMBIES_API UPlayerUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void PullWidgetClassesFromGameInstance();
	void SetOwnerPC(AMyPlayerController* InPC);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UMenuUIManager* GetMenuManager();

	UFUNCTION(BlueprintCallable, Category = "UI") void ShowPauseMenu();
	UFUNCTION(BlueprintCallable, Category = "UI") void HidePauseMenu();
	UFUNCTION(BlueprintCallable, Category = "UI") void ShowDeathScreenLocal();
	UFUNCTION(BlueprintCallable, Category = "UI") void ShowRoundIntroSplashWidget(int32 RoundNumber);
	UFUNCTION(BlueprintCallable, Category = "UI") void HideRoundIntroSplash();
	UFUNCTION(BlueprintCallable, Category = "UI") void PlayRoundIntroSound();
	UFUNCTION(BlueprintCallable, Category = "UI") void UpdateRoundHUD(int32 RoundNumber);
	UFUNCTION(BlueprintCallable, Category = "UI") void HideRoundHUD();

	UFUNCTION(BlueprintPure, Category = "UI")
    bool IsPauseMenuVisible() const;

private:
	EInputProfile GetDesiredProfileForCurrentMap() const;

	void ShowOverlayInternal(
		TSubclassOf<UUserWidget> WidgetClass,
		UUserWidget*& InOutInstance,
		int32 ZOrder,
		bool bPauseWorld,
		bool bMultiplayerConfig,
		EOverlayInputMode InputMode);

	template <typename T>
	void ShowOverlay(
		TSubclassOf<T> WidgetClass,
		TObjectPtr<T>& OutInstance,
		int32 ZOrder = 1000,
		bool bPauseWorld = false,
		bool bMultiplayerConfig = false,
		EOverlayInputMode InputMode = EOverlayInputMode::None)
	{
		UUserWidget* Temp = OutInstance.Get();
		ShowOverlayInternal(WidgetClass, Temp, ZOrder, bPauseWorld, bMultiplayerConfig, InputMode);
		OutInstance = Cast<T>(Temp);
	}

	void HideOverlay(UUserWidget* WidgetInstance, bool bRestoreInput);

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AMyPlayerController> OwnerPC;

	UPROPERTY(Transient)
	TObjectPtr<UMenuUIManager> MenuUI;

	UPROPERTY() TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY() TSubclassOf<UUserWidget> SoloMenuWidgetClass;
	UPROPERTY() TSubclassOf<UUserWidget> GameModeSelectionMenuWidgetClass;
	UPROPERTY() TSubclassOf<UUserWidget> CreateSessionMenuWidgetClass;
	UPROPERTY() TSubclassOf<UUserWidget> JoinSessionMenuWidgetClass;
	UPROPERTY() TSubclassOf<UUserWidget> MultiplayerMenuWidgetClass;

	UPROPERTY() TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;
	UPROPERTY() TSubclassOf<UZombiesRoundWidget> RoundSplashWidgetClass;
	UPROPERTY() TSubclassOf<UGameOverMenuWidget> DeathScreenWidgetClass;
	UPROPERTY() TSubclassOf<UZombiesRoundWidget> RoundHUDWidgetClass;

	UPROPERTY(Transient) TObjectPtr<UPauseMenuWidget> PauseMenuInstance = nullptr;
	UPROPERTY(Transient) TObjectPtr<UGameOverMenuWidget> DeathScreenInstance = nullptr;
	UPROPERTY(Transient) TObjectPtr<UZombiesRoundWidget> RoundHUDWidgetInstance = nullptr;
	UPROPERTY(Transient) TObjectPtr<UZombiesRoundWidget> RoundSplashWidgetInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> RoundIntroThudComp = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> RoundThudSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta=(ClampMin="0.0"))
	float RoundIntroWidgetDuration = 2.0f;

	int32 LastIntroSoundRoundPlayed = INDEX_NONE;
	FTimerHandle RoundIntroHideTimerHandle;
};