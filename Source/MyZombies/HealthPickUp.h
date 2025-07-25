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
UCLASS()
class MYZOMBIES_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()

public:
	// Create constructor
	AHealthPickUp();

	/*
		This class will have its own behavior for inherited OnSphereBeginOverlap function
		So we will call parent version and add functionality to it
	*/
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	void AddHealth(float PlayerHealth, float MaxHealth);

	void ShowPickUpWidget(bool bShowWidget);
	

private:

	class AMainCharacter* MainCharacter;

    UWidgetComponent* HealthPickUpWidget;
	
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* NiagaraHealthComponent;

	
	class AMyPlayerController* myPlayerController;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	// Amount of health to add per pick up
	UPROPERTY(EditAnywhere)
	int32 HealthAdd = 30;
};
