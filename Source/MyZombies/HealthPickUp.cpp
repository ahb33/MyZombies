// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickUp.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MainCharacter.h"
#include "MyPlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"


AHealthPickUp::AHealthPickUp()
{
   	// make sure this pick up class replicates
	bReplicates = true;
	SetReplicateMovement(true); 

    NiagaraHealthComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraHealthComponent"));
    NiagaraHealthComponent->SetupAttachment(RootComponent);
	NiagaraHealthComponent->SetVisibility(false);

}

bool AHealthPickUp::TryConsume(APawn* ByPawn)
{
	if (!HasAuthority()) return false;
	AMainCharacter* MC = Cast<AMainCharacter>(ByPawn);
	if (!MC) return false;
	UE_LOG(LogTemp, Warning, TEXT("Health before pick up is %f"), MC->GetPlayerHealth());

	const float NewH = FMath::Clamp(MC->GetPlayerHealth() + (float)HealthAdd, 0.f, MC->GetMaxHealth());
	UE_LOG(LogTemp, Warning, TEXT("Health is now %f"), NewH);
	
	MC->SetPlayerHealth(NewH); 

	Multicast_PlayPickupEffect(GetActorLocation());
	Destroy();
	return true;
}

void AHealthPickUp::Multicast_PlayPickupEffect_Implementation(const FVector& AtLocation)
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, AtLocation);
	}
}
