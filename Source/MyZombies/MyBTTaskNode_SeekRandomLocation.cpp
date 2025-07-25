// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBTTaskNode_SeekRandomLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"


EBTNodeResult::Type
UMyBTTaskNode_SeekRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemoery)
{
	// Get blackboard Component
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if(BlackboardComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// Get controlled pawn that is running this instance of the Behavior Tree
	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn) 
	{
		return EBTNodeResult::Failed;
	}

	//Get Navigation system
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	//Prepare variables for Query
	FNavLocation Result;
	FVector Origin = ControlledPawn->GetActorLocation();

	// Perform Query
	bool bSuccess = NavSys->GetRandomReachablePointInRadius(Origin, Radius, Result);
	if (!bSuccess)
	{
		return EBTNodeResult::Failed;
	}

	// Save result and return success
	BlackboardComp->SetValueAsVector(DestinationVector.SelectedKeyName, Result.Location);
	return EBTNodeResult::Succeeded;

}

FString UMyBTTaskNode_SeekRandomLocation::GetStaticDescription() const
{
	// Retrieve the key name or an empty string if not set
	FString KeyName = DestinationVector.IsSet() ? DestinationVector.SelectedKeyName.ToString() : TEXT("");

	// Format the description with the destination key and radius
	return FString::Printf(TEXT("DestinationKey: '%s'\nRadius: '%s'"), *KeyName, *FString::SanitizeFloat(Radius));
}