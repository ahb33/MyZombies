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

    /** Flip ready flag (client or server) */
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetReadyStatus(bool bIsReady);

    /** Direct check of ready flag */
    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool IsReady() const { return bReady; }

protected:
    /** Server RPC to set ready state */
    UFUNCTION(Server, Reliable)
    void ServerSetReadyStatus(bool bIsReady);

    /** Replicated flag, notifies on change */
    UPROPERTY(ReplicatedUsing = OnRep_Ready)
    bool bReady;

    /** Called on clients when bReady updates */
    UFUNCTION()
    void OnRep_Ready();

    /** Shared setter logic */
    void UpdateReadyStatus(bool bIsReady);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};