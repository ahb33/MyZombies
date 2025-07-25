#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    ECS_Reloading UMETA(DisplayName = "Reloading"),
    ECS_Equipping UMETA(DisplayName = "Equipping"),
    ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
/*
add code so that you dont move while reloading
*/