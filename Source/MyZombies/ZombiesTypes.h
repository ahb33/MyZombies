#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
    Intro        UMETA(DisplayName="Intro"),
    Active       UMETA(DisplayName="Active"),
    Intermission UMETA(DisplayName="Intermission"),
    End          UMETA(DisplayName="End")
};


// // use to add functionality 
// UENUM(BlueprintType)
// enum class EPlayerLifeState : uint8
// {
//     Alive  UMETA(DisplayName="Alive"),
//     Downed UMETA(DisplayName="Downed"),
//     Dead   UMETA(DisplayName="Dead"),
// };
