// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageQuery.h" 
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "AIController.h"
 
 
 static void GetActorTags(const AActor* Actor, FGameplayTagContainer& Out)
 {
    if (!Actor) return;
    if (const IGameplayTagAssetInterface* TagSrc = Cast<IGameplayTagAssetInterface>(Actor))
    {
    TagSrc->GetOwnedGameplayTags(Out);
    }
 }

bool UDamageQuery::ActorHasTag(const AActor* Actor, const FGameplayTag& Tag)
{
    FGameplayTagContainer Tags;
    GetActorTags(Actor, Tags);
    return Tags.HasTag(Tag);
}

 EUnitKind UDamageQuery::GetUnitKind(const AActor* Actor)
 {
    if (!Actor) return EUnitKind::Unknown;
    
    static const FGameplayTag TagPlayer = FGameplayTag::RequestGameplayTag(TEXT("Unit.Player"));
    static const FGameplayTag TagAI = FGameplayTag::RequestGameplayTag(TEXT("Unit.AI"));
    
    FGameplayTagContainer Tags;
    GetActorTags(Actor, Tags);
    
    if (Tags.HasTag(TagPlayer)) return EUnitKind::Player;
    if (Tags.HasTag(TagAI)) return EUnitKind::AI;
    
    const APawn* Pawn = Cast<APawn>(Actor);
    if (Pawn && Pawn->IsPlayerControlled()) return EUnitKind::Player;
    if (Cast<APawn>(Actor)) return EUnitKind::AI;
    
    return EUnitKind::Unknown;
 }

 uint8 UDamageQuery::GetTeamId(const AActor* Actor)
 {
    if (!Actor) return 255;
    
    // Prefer Gameplay Tags "Team.*"
    FGameplayTagContainer Tags;
    GetActorTags(Actor, Tags);
    for (const FGameplayTag& T : Tags)
    {
        if (T.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Team"))))
        {
        // Map the last segment to a small ID (hash).
        const FString Name = T.GetTagName().ToString(); // e.g., "Team.Alpha"
        const int32 Dot = Name.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        const FString Leaf = Dot != INDEX_NONE ? Name.Mid(Dot + 1) : Name;
        return static_cast<uint8>(FCrc::StrCrc32(*Leaf)); // deterministic small ID
        }
    }
 
    // Fallback to GenericTeam
    if (const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(Actor))
    {
    return TeamAgent->GetGenericTeamId().GetId();
    }
    if (const APawn* Pawn = Cast<APawn>(Actor))
    {
    if (const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(Pawn->GetController()))
    {
    return TeamAgent->GetGenericTeamId().GetId();
    }
    }
    return 255; // Unknown
 }