// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"


// fix redundancies code
// Sets default values
APickUp::APickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// make sure this pick up class replicates
	bReplicates = true;
	SetReplicateMovement(true);

	USceneComponent* RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpItems"));
	PickUpMesh->SetupAttachment(RootSceneComponent);
	PickUpMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
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

}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
	// Check if has authority(this is a replicated actor, you only want to overlap events on server)
	// Bind begin and end OnSphereOverlap functions to ItemOverlapSphere
	ItemOverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereBeginOverlap);
	
}

// Called every frame
void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickUp::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

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
		PickUpWidget->SetVisibility(false);
	}
}

void APickUp::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
}