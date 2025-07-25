#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    EWT_Shotgun UMETA(DisplayName = "Shotgun"),

    EWT_None UMETA(DisplayName = "None"), // Add this to represent no weapon

};