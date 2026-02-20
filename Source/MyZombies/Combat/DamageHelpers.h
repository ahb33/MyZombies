#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "BaseGameState.h"



namespace DamageHelpers
{
    [[nodiscard]] inline const APawn* ResolveKillerPawn(AController* Inst, AActor* Causer)
    {
        if (Inst && Inst->GetPawn()) return Inst->GetPawn();
        if (Causer)
        {
            if (APawn* P = Causer->GetInstigator()) return P; // projectile/weapon path
            if (AController* C = Causer->GetInstigatorController()) return C->GetPawn();
        }
        return nullptr;
    }


    [[nodiscard]] inline const AActor* ResolveKillerActor(AController* Inst, AActor* Causer)
    {
        if (const APawn* P = ResolveKillerPawn(Inst, Causer)) return P;
        return Causer;
    }


    [[nodiscard]] inline bool IsZombiesMode(const UWorld* W)
    {
        const ABaseGameState* GS = W ? W->GetGameState<ABaseGameState>() : nullptr;
        return GS && GS->GetMatchMode() == EMatchMode::Zombies;
    }

    [[nodiscard]] inline bool IsZombieActor(const AActor* A)
    {
        if (!A) return false;
        const FGameplayTag ZombieTag = FGameplayTag::RequestGameplayTag(TEXT("Faction.Zombie"), false);
        if (!ZombieTag.IsValid()) return false;

        if (const IGameplayTagAssetInterface* I = Cast<IGameplayTagAssetInterface>(A))
        {
            FGameplayTagContainer Tags;
            I->GetOwnedGameplayTags(Tags);
            return Tags.HasTag(ZombieTag);
        }
        return false;
    }
} 