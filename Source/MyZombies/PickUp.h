// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "PickUp.generated.h"


// forward-declares:
class USphereComponent; 
UCLASS()
class MYZOMBIES_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUp();

	virtual void Destroyed() override;

	void SetPickUpWidget(UUserWidget* Widget);
	UWidgetComponent* GetPickUpWidget() const {return PickUpWidget;}
	void ShowPickUpWidget(bool bShowWidget);

	/** Server-only; return true if consumed and should be destroyed. */
	virtual bool TryConsume(APawn* ByPawn);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UPROPERTY(VisibleAnywhere, Category = Item)
	TObjectPtr<UStaticMeshComponent> PickUpMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Item)
	TObjectPtr<USphereComponent> ItemOverlapSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Item)
	TObjectPtr<UWidgetComponent> PickUpWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = Item)
	TObjectPtr<USoundCue> PickupSound = nullptr;

	bool bConsumed = false;
};
