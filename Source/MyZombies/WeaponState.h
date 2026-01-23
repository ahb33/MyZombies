#pragma once

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Equipped UMETA(DisplayName = "Equipped"),

	Unequipped UMETA(DisplayName = "Unequipped"),

	EquippedSecondary UMETA(DisplayName = "Equipped"),
	Dropped UMETA(DisplayName = "Dropped"),

	MAX UMETA(DisplayName = "DefaultMAX")
};
