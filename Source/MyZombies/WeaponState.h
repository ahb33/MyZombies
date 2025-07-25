#pragma once

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Equipped UMETA(DisplayName = "Equipped"),

	EWS_Unequipped UMETA(DisplayName = "Unequipped"),

	EWS_EquippedSecondary UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
