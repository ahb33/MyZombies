// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "GenericTeamAgentInterface.h"
#include "DamageQuery.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class EUnitKind : uint8
{
    Unknown = 0,
    Player  = 1,
    AI      = 2
};

UCLASS()
class MYZOMBIES_API UDamageQuery : public UObject
{
	GENERATED_BODY()

public:

	/** Reads UnitKind from tags: Unit.Player / Unit.AI; falls back to controller type. */
 	UFUNCTION(BlueprintPure, Category="Damage")
 	static EUnitKind GetUnitKind(const AActor* Actor);

	/** Reads Team from tags Team.* (first match) or IGenericTeamAgentInterface; Unknown=255. */
	UFUNCTION(BlueprintPure, Category="Damage")
	static uint8 GetTeamId(const AActor* Actor);
 
	/** Has tag helper (works with actors that expose tags). */
	UFUNCTION(BlueprintPure, Category="Damage")
	static bool ActorHasTag(const AActor* Actor, const FGameplayTag& Tag);
 };
