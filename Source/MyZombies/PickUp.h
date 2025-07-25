// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "PickUp.generated.h"

UCLASS()
class MYZOMBIES_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUp();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	void SetPickUpWidget(UUserWidget* Widget);
	UWidgetComponent* GetPickUpWidget() const {return PickUpWidget;}

	void ShowPickUpWidget(bool bShowWidget);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// you only want overlap events to happen on the server


private:

	UPROPERTY(VisibleAnywhere, Category = Item)
	class UStaticMeshComponent* PickUpMesh;


	//unlike in weapon class we cant overslap sphere 
	UPROPERTY(EditAnywhere, Category = Item)
	class USphereComponent* ItemOverlapSphere;


	UPROPERTY(EditAnywhere, Category = Item)
	class UWidgetComponent* PickUpWidget;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;


};
