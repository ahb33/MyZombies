// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BTService_ChasingBehavior.generated.h"

/**
 * 
 */


class AActor;

UCLASS()
class MYZOMBIES_API UBTService_ChasingBehavior : public UBTService
{
	GENERATED_BODY()

public:
	// Constructor used to initialize values declared in header;
	UBTService_ChasingBehavior(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


private:

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector CanSeePlayerKey;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector CanHearPlayerKey;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector PlayerKey;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector LastKnownPositionKey;

	bool bLastCanSeePlayer;

	UPROPERTY(EditAnywhere, Category = Player)
	TSubclassOf<AActor> PlayerClass;


protected:
	// Behavior Tree tick function
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override;


};