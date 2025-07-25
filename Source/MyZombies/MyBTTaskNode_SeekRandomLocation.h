// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MyBTTaskNode_SeekRandomLocation.generated.h"

/**
 * 
 */
UCLASS()
class MYZOMBIES_API UMyBTTaskNode_SeekRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector DestinationVector; // this will hold reference to the newly calculated desintation; needs to be accessible within blackboard

	UPROPERTY(EditAnywhere, Category = Blackboard)
	float Radius = 300.f; // needs to be accessible within blackboard

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override;


	
};
