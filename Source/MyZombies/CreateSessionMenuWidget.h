#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateSessionMenuWidget.generated.h"


class UButton;                 // bound widget
class UComboBoxString;         // bound widget
class UMultiplayerSessions;    // subsystem (defined elsewhere)

/**
 change name to CreateSessionMenu
 */
UCLASS()
class MYZOMBIES_API UCreateSessionMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnCreateSessionClicked();

    UFUNCTION()
    void OnCreateSessionComplete(bool bWasSuccessful);

    int32 GetSelectedPlayerCount() const;
    FString GetSelectedGameType() const;

protected:
    // Bound UMG widgets (forward-declared types are fine for UPROPERTY pointers)
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* CreateSessionButton = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UComboBoxString* NumOfPlayersBox = nullptr;

private:
    // Subsystem pointer; forward-declared + UPROPERTY is fine
    UPROPERTY()
    UMultiplayerSessions* MultiplayerSubsystem = nullptr;
};
