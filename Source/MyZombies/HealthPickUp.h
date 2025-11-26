// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "NiagaraComponent.h"
#include "HealthPickUp.generated.h"


/**
 Class needs to now how much to heal when we pick up
 Coud add variable to see health bar filling up
 Could add naigara component to use the ones we download
 Could spawn effect when health pick up is destroyed; call parent version and add functionality
 Will need niagara system not component


 */


class UWidgetComponent;
class AMainCharacter;
class UNiagaraComponent;

UCLASS()
class MYZOMBIES_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()

public:
	// Create constructor
	AHealthPickUp();

	virtual bool TryConsume(APawn* ByPawn) override;
private:
	UPROPERTY(VisibleAnywhere, Category="VFX")
	TObjectPtr<UNiagaraComponent> NiagaraHealthComponent = nullptr;

	class AMyPlayerController* myPlayerController = nullptr;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<UNiagaraSystem> PickupEffect = nullptr;

	// Amount of health to add per pick up
	UPROPERTY(EditAnywhere, Category="Health")
	int32 HealthAdd = 30;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPickupEffect(const FVector& AtLocation);

};