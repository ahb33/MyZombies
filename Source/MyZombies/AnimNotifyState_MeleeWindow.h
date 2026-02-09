// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MeleeWindow.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UAnimNotifyState_MeleeWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Melee")
	FName StrikeSocket = TEXT("hand_r");         

	UPROPERTY(EditAnywhere, Category="Melee")
	float SphereRadius = 15.f; // cm

	UPROPERTY(EditAnywhere, Category="Melee")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& Ref) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& Ref) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Ref) override;

private:

	// Per-mesh state
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FVector> PrevLoc;

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TSet<TWeakObjectPtr<AActor>>> AlreadyHit;
	
};
