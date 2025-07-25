// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"
#include "MainCharacter.h"
#include "Weapon.h"

void AAmmoPickUp::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    /*
    Create pointer to Weapon class
    Construct variable that is pointer to MainCharacter and cast OtherActor from Sphere begin overlap
    Check if MainCharacter is valid
    Construct myWeapon variable that is pointer to Weapon class and call GetPrimaryWeapon within MainCharacter
    Check if myWeapon is valid

    */

    MainCharacter = Cast<AMainCharacter>(OtherActor);
    if(MainCharacter)
    {
        // myWeapon = MainCharacter->GetEquippedWeapon();
        // if (myWeapon)
        // {
            
        //     // myWeapon->PickUpAmmo(WeaponType, AmmoAmount);
        // }

    }

    Destroy();

}

int32 AAmmoPickUp::GetAmmoAmountForWeaponType( )
{
	switch (WeaponType)
	{
	case EWeaponType::EWT_AssaultRifle:
		return 50; // example amount for Assault Rifle
	case EWeaponType::EWT_Shotgun:
		return 8;  // example amount for Shotgun
	default:
		return 0;
	}
}
