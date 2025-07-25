// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickUp.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MainCharacter.h"
#include "MyPlayerController.h"
#include "Components/WidgetComponent.h"


AHealthPickUp::AHealthPickUp()
{
   	// make sure this pick up class replicates
	bReplicates = true;
	SetReplicateMovement(true); 

    NiagaraHealthComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraHealthComponent"));
    NiagaraHealthComponent->SetupAttachment(RootComponent);

    // Construct Pick up Widget and Niagara Health Component and attach to root
    HealthPickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthPickupWidget"));
	HealthPickUpWidget->SetupAttachment(RootComponent);
	HealthPickUpWidget->SetVisibility(false);
}

void AHealthPickUp::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMainCharacter* LocalMainCharacter = Cast<AMainCharacter>(OtherActor);
	if(LocalMainCharacter && HealthPickUpWidget)
	{
		LocalMainCharacter->SetOverlappingItem(this);
	}
}

void AHealthPickUp::AddHealth(float CurrentHealth, float MaxHealth )
{
    UE_LOG(LogTemp, Warning, TEXT("Add Health function called within HealthPickUp class"));
    // Check if MainCharacter is valid
    // Construct Player Controller
    // Get value for Updated Health after health pick up
    // Clamp it between 0 and MaxHealth
    // Update HUD in player controller

    myPlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());

    if (myPlayerController)
    {
        float UpdatedHealth = FMath::Clamp(CurrentHealth + HealthAdd, 0.0f, MaxHealth);
        myPlayerController->SetHUDHealth(UpdatedHealth, MaxHealth);
    } 

    Destroy();
}

void AHealthPickUp::ShowPickUpWidget(bool bShowWidget)
{
    if (HealthPickUpWidget)
    {
        HealthPickUpWidget->SetVisibility(bShowWidget);
    }
}