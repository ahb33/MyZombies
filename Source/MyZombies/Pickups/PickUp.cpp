// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "MainCharacter.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"


// fix redundancies code
// Sets default values
APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = false;

	// make sure this pick up class replicates
	bReplicates = true;
	SetReplicateMovement(true);

	USceneComponent* RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpItems"));
	PickUpMesh->SetupAttachment(RootSceneComponent);
	PickUpMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); /*
	if you want the pawn to run through the weapon without colliding with it */
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	// Construct ItemOverlapSphere
	// Attach it to PickUpMesh Root Component
	// Set the radius of the ItemOverlapSphere to 150 units(adjust later)
	//	Set collision response channels

	ItemOverlapSphere = CreateDefaultSubobject<USphereComponent>("ItemOverlapSphere");
	ItemOverlapSphere->SetupAttachment(RootComponent); 
	ItemOverlapSphere->SetSphereRadius(150.f);
	// Enable collision detection for the item overlap sphere, but set it to only respond to queries (no physics simulation)
	ItemOverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Ignore collision with all channels by default
	ItemOverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// Set item overlap sphere to overlap with pawns only
	ItemOverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
    PickUpWidget->SetupAttachment(RootComponent);
	PickUpWidget->SetVisibility(false);

}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
	// Check if has authority(this is a replicated actor, you only want to overlap events on server)
	// Bind begin and end OnSphereOverlap functions to ItemOverlapSphere
	ItemOverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereBeginOverlap);
	ItemOverlapSphere->OnComponentEndOverlap.AddDynamic(this, &APickUp::OnSphereEndOverlap);

	ShowPickUpWidget(false);
	
}



void APickUp::OnSphereBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    AMainCharacter* MC = Cast<AMainCharacter>(OtherActor);
    if (!MC) return;

    MC->SetOverlappingItem(this);


}

void APickUp::OnSphereEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	
	if (!IsValid(this)) return;
    if (!IsValid(OtherActor)) return;
	AMainCharacter* MC = Cast<AMainCharacter>(OtherActor);
	if (!MC) return;

	MC->SetOverlappingItem(nullptr);
}
void APickUp::SetPickUpWidget(UUserWidget* Widget)
{
	// Construct Pickup Widget and attach to root component
	PickUpWidget->SetWidget(Widget);
}

void APickUp::ShowPickUpWidget(bool bShowWidget)
{
	if(PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

bool APickUp::TryConsume(APawn* /*ByPawn*/)
{
	// Base does nothing.
	return false;
}

void APickUp::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
}