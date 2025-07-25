#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AICharacterStats.generated.h"

USTRUCT(BlueprintType)
struct FAICharacterStats : public FTableRowBase
{
    GENERATED_BODY()

    // AI Health multiplier (e.g., affects health scaling)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Stats")
    float HealthMultiplier = 1.0f;

    // AI Speed multiplier (e.g., affects movement speed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Stats")
    float SpeedMultiplier = 1.0f;

    // AI Damage multiplier (e.g., affects damage dealt to players)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Stats")
    float DamageMultiplier = 1.0f;

    // Default value for the number of zombies per level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    int32 ZombiesPerLevel = 10; 
};