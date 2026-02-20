#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    Shotgun UMETA(DisplayName = "Shotgun"),

    None UMETA(DisplayName = "None") // Add this to represent no weapon

};