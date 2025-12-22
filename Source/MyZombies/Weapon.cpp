// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"  
#include "MainCharacter.h"        
#include "GameFramework/Actor.h"             
#include "MyPlayerController.h" 
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"            
#include "Net/UnrealNetwork.h"                

// Sets default values
AWeapon::AWeapon()
    : Damage(0.f),        
      AmmoOnHand(0),
      AmmoInMag(0),
      MagCapacity(0),
      MaxAmmoOnHand(0)
{
    // Enable replication and ticking
    bReplicates = true;
    SetReplicateMovement(true);
    PrimaryActorTick.bCanEverTick = true;

    // Weapon Mesh initialization
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh); // Set WeaponMesh as root component.
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Area Sphere initialization
    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(RootComponent);
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Pickup Widget initialization
    PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
    PickupWidget->SetupAttachment(RootComponent);
    PickupWidget->SetVisibility(false);

    // Persistent PSC; template set in BeginPlay to allow BP overrides of Tracer.
    TracerSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TracerPSC"));
    TracerSystem->SetupAttachment(WeaponMesh, GetMuzzleSocketName());
    TracerSystem->bAutoActivate = false;         // manual fire control
    TracerSystem->bAutoDestroy  = false;         // keep it alive

    MaxTraceDistance = 80000.f;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    
	/*
        We'd like ammo on hand and the ammo in the magazine to be replicated on all clients in case one player picked up a used weapon
	    then you want the ammo count on that weapon to remain unchanged
	*/
	DOREPLIFETIME_CONDITION(AWeapon, AmmoOnHand, COND_OwnerOnly);  // Ammo will only replicate to owning client
	DOREPLIFETIME_CONDITION(AWeapon, AmmoInMag, COND_OwnerOnly);
    DOREPLIFETIME(AWeapon, WeaponState);
    DOREPLIFETIME(AWeapon, WeaponType);
    
}
// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();


	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	}
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
    if(HasAuthority())
    {
        SetAmmoInMag();
        SetAmmoOnHand();
    }
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



}				

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMainCharacter* LocalMainCharacter = Cast<AMainCharacter>(OtherActor);
	if(LocalMainCharacter && PickupWidget)
	{
		LocalMainCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMainCharacter* LocalMainCharacter = Cast<AMainCharacter>(OtherActor);
	if(LocalMainCharacter)
	{
		LocalMainCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::DropWeapon()
{
    // Detach the weapon from the character's hand socket
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Set the weapon's owner to nullptr
    SetOwner(nullptr);

    // Show the pickup widget for the dropped weapon
    ShowPickUpWidget(true);
}

void AWeapon::Fire(const FVector& HitTargetParam)
{
    if (!GetOwner() || !GetWeaponMesh())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot fire: Weapon is empty or invalid owner/mesh."));
        return;
    }
    
    RoundFired();

    FVector NoiseLocation = GetActorLocation(); // or muzzle socket
    AActor* NoiseInstigator = Cast<AActor>(GetOwner());
    const float Loudness = 1.5f;   // tweak per weapon
    const float MaxRange = 3000.f; // cm
    const FName Tag = TEXT("Gunfire");

    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        NoiseLocation,
        Loudness,
        NoiseInstigator,
        MaxRange,
        Tag
    );
}

void AWeapon::DealDamage(const FHitResult &HitResult)
{
    if (AActor* HitActor = HitResult.GetActor())
    {        
        UGameplayStatics::ApplyDamage(
            HitActor,
            GetDamage(),
            GetOwner()->GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );
    }
}

bool AWeapon::WeaponIsEmpty() const
{
    return GetCurrentAmmoInMag() == 0;
}

void AWeapon::RoundFired()
{
    if(!HasAuthority()) return;

    const int32 Mag = GetCurrentAmmoInMag();
    if (Mag > 0)
    {
        SetAmmo(GetCurrentAmmoOnHand(), Mag - 1);
        RefreshHUD();
    }
}


void AWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::RefreshHUD()
{
    if (MainCharacter)
    {
        AMyPlayerController* PlayerController = Cast<AMyPlayerController>(MainCharacter->GetController());
        if (PlayerController)
        {
            PlayerController->SetHUDAmmo(GetCurrentAmmoOnHand());
            PlayerController->SetHUDMagAmmo(GetCurrentAmmoInMag());
        }
    }
}


void AWeapon::ReloadAmmo(int32 Requested)
{
    if (!HasAuthority()) return; // server-only mutation

    const int32 Mag      = GetCurrentAmmoInMag();                 // in-mag rounds
    const int32 Reserve  = GetCurrentAmmoOnHand();                // backpack/extra
    const int32 Capacity = GetMagCapacity();
    const int32 Space    = FMath::Max(0, Capacity - Mag);
    const int32 ToLoad   = FMath::Clamp(Requested, 0, FMath::Min(Space, Reserve));

    // Correct order: SetAmmo(OnHand, InMag)
    SetAmmo(Reserve - ToLoad, Mag + ToLoad);
    RefreshHUD();
}

void AWeapon::SetHUDAmmo(int32 Ammo)
{
    if (MainCharacter)
    {
        AMyPlayerController* PlayerController = Cast<AMyPlayerController>(MainCharacter->GetController());
        if (PlayerController)
        {
            PlayerController->SetHUDAmmo(Ammo);
        }
    }
}

void AWeapon::SetHUDMagAmmo(int32 MagAmmo)
{
    if (MainCharacter)
    {
        AMyPlayerController* PlayerController = Cast<AMyPlayerController>(MainCharacter->GetController());
        if (PlayerController)
        {
            PlayerController->SetHUDMagAmmo(MagAmmo);
        }
    }
}

void AWeapon::SetOwner(AActor* NewOwner)
{
    Super::SetOwner(NewOwner);
    MainCharacter = Cast<AMainCharacter>(NewOwner);
    if (MainCharacter)
    {
        RefreshHUD();
    }
    else
    {
        MainCharacter = nullptr;
    }
}

void AWeapon::OnRep_AmmoOnHand()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_AmmoOnHand: AmmoOnHand = %d"), AmmoOnHand);
    RefreshHUD();
}

void AWeapon::OnRep_AmmoInMag()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_AmmoInMag: AmmoInMag = %d"), AmmoInMag);
    RefreshHUD();
}

void AWeapon::SetAmmoInMag()
{
	AmmoInMag = GetCurrentAmmoInMag();
}

void AWeapon::SetAmmoOnHand()
{
	AmmoOnHand = GetCurrentAmmoOnHand();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
    if (GetOwner() == nullptr)
    {
        MainCharacter = nullptr;
    }
    else
    {
        MainCharacter = Cast<AMainCharacter>(GetOwner());
        if (MainCharacter)
        {
            // Initialize any other necessary references
            RefreshHUD();
        }
    }
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
    OnRep_WeaponState();

}
void AWeapon::OnRep_WeaponState()
{

	switch(WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetEnableGravity(false);
			// Perform additional actions when the weapon is equipped
			break;

		case EWeaponState::EWS_EquippedSecondary:
			// could refactor block of code below and above into one function -
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetEnableGravity(false);
            break;

		case EWeaponState::EWS_Dropped:
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
			// Perform additional actions when the weapon is unequipped
			break;
	}
}

void AWeapon::UpdateTracer(const FVector& SourceWS, const FVector& TargetWS)
{
    if (!TracerSystem) return;
    TracerSystem->SetBeamSourcePoint(TracerEmitterIndex, SourceWS, /*PointIndex=*/0);
    TracerSystem->SetBeamTargetPoint(TracerEmitterIndex, TargetWS, /*PointIndex=*/0);
    TracerSystem->ActivateSystem(true);          // restart for per-shot burst
}

void AWeapon::PlayFireEffects(const FHitResult& Hit, const FVector& Start, const FVector& End)
{
    static const FName MuzzleName("MuzzleFlash");
    const FVector MuzzleLoc = GetWeaponMesh()->GetSocketLocation(MuzzleName);

    if (UParticleSystem* MF = GetMuzzleFlash())
    {
        // Prefer attached so it follows weapon recoil/animations
        UGameplayStatics::SpawnEmitterAttached(MF, GetWeaponMesh(), MuzzleName);
    }
    
    // Compute the visual end we want for the tracer.
    const FVector VisualEnd = Hit.bBlockingHit ? Hit.ImpactPoint : End;

    if (TracerSystem)
    {
        UpdateTracer(MuzzleLoc, VisualEnd);
    }

    else if (UParticleSystem* TracerFX = GetTracer())
    {
        // Fallback: one-shot spawned tracer aligned along path (no live beam control).
        const FRotator DirRot = (VisualEnd - MuzzleLoc).Rotation();
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, Start, (VisualEnd - Start).Rotation());
    }

    // Impact FX only when we actually hit something
    if (Hit.bBlockingHit)
    {
        if (USoundCue* IS = GetImpactSound())
        {
            UGameplayStatics::PlaySoundAtLocation(this, IS, Hit.ImpactPoint);
        }
        if (UParticleSystem* IP = GetImpactParticles())
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), IP, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
        }
    }
    // add tracer beam
}

void AWeapon::Multicast_PlayFireFX_Implementation(const FVector& TraceStart, const FVector& TraceEnd)
{
    
}
