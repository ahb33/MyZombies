#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    ALobbyPlayerState();

    UFUNCTION(BlueprintCallable)
    void SetReadyStatus(bool bIsReady);

    UFUNCTION(BlueprintCallable)
    bool IsReady() const { return bReady; }

    UFUNCTION(BlueprintCallable)
    void SetTeamID(int32 NewTeamID) { TeamID = NewTeamID; }

    UFUNCTION(BlueprintCallable)
    int32 GetTeamID() const { return TeamID; }

protected:
    UPROPERTY(ReplicatedUsing=OnRep_ReadyStatus, BlueprintReadOnly, Category = "Lobby")
    bool bReady;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
    int32 TeamID;

    UFUNCTION()
    void OnRep_ReadyStatus();  // Called when bReady changes
	
};