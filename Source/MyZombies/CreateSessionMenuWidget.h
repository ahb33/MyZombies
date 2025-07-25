#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerSessions.h"
#include "CreateSessionMenuWidget.generated.h"

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
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    class UButton* CreateSessionButton;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    class UComboBoxString* NumOfPlayersBox;

private:
    class UMultiplayerSessions* MultiplayerSubsystem;
	
};
