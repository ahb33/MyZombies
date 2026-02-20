// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ChasingBehavior.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyCharacter.h"
#include "MainCharacter.h"
#include "MyAIController.h"

namespace
{
	static const FName BB_CanSeePlayer(TEXT("CanSeePlayer"));
	static const FName BB_CanHearPlayer(TEXT("CanHearPlayer"));
	static const FName BB_Player(TEXT("Player"));
	static const FName BB_LastKnownPosition(TEXT("LastKnownPosition"));
	static const FName BB_PlayerWithinRange(TEXT("PlayerWithinRange"));

	FName ResolveKey(const FBlackboardKeySelector& Selector, const FName Fallback)
	{
		return (Selector.SelectedKeyName != NAME_None) ? Selector.SelectedKeyName : Fallback;
	}
}

UBTService_ChasingBehavior::UBTService_ChasingBehavior(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // Set the update interval for the behavior
    bNotifyTick = true;
    Interval = 0.5f; // Adjust the interval as needed
    RandomDeviation = 0.1f;

    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = false;

    // Filter the blackboard key selectors
    PlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ChasingBehavior, PlayerKey), AActor::StaticClass());
	LastKnownPositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ChasingBehavior, LastKnownPositionKey));
	CanSeePlayerKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ChasingBehavior, CanSeePlayerKey));
	CanHearPlayerKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ChasingBehavior, CanHearPlayerKey));
	PlayerWithinAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ChasingBehavior, PlayerWithinAttackRangeKey));
}



void UBTService_ChasingBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    GetStaticDescription();

    UBlackboardComponent* BB  = OwnerComp.GetBlackboardComponent();
    if (BB  == nullptr) return;

    // Get the AI controller from OwnerComp and check whether it is valid
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC == nullptr) return;
         
    // cast our AI controller to our custom AI controller
    AMyAIController* ZombieAI = Cast<AMyAIController>(AIC);
    if (ZombieAI == nullptr) return;

    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AIC->GetPawn());
	if (!Enemy) return;


    
	const FName CanSeeKeyName = ResolveKey(CanSeePlayerKey, BB_CanSeePlayer);
	const FName CanHearKeyName = ResolveKey(CanHearPlayerKey, BB_CanHearPlayer);
	const FName PlayerKeyName = ResolveKey(PlayerKey, BB_Player);
	const FName LKPKeyName = ResolveKey(LastKnownPositionKey, BB_LastKnownPosition);
	const FName InRangeKeyName = ResolveKey(PlayerWithinAttackRangeKey, BB_PlayerWithinRange);

	const bool bCanSeePlayer = BB->GetValueAsBool(CanSeeKeyName);
	const bool bCanHearPlayer = BB->GetValueAsBool(CanHearKeyName);

    AActor* PlayerActor = Cast<AActor>(BB->GetValueAsObject(PlayerKeyName));
	AMainCharacter* Player = Cast<AMainCharacter>(PlayerActor);


 
    // Update the last known player position based on the current perception
	if (bCanSeePlayer && PlayerActor)
	{
		BB->SetValueAsVector(LKPKeyName, PlayerActor->GetActorLocation());
	}

	// Keep "within range" fresh; perception events are NOT enough for this.
	const bool bInRange = (Player && Enemy) ? ZombieAI->IsPlayerWithinRange(Player, Enemy) : false;
	BB->SetValueAsBool(InRangeKeyName, bInRange);

	if (bInRange)
	{
		AIC->StopMovement();
	}


	bLastCanSeePlayer = bCanSeePlayer;
    
}



// Called when auxiliary node becomes active; this function should be considered as const (doesn't modify state of object) if node is not instanced!
void UBTService_ChasingBehavior::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    // Get reference to the player so that we can store it on the blackboard
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	const FName PlayerKeyName = ResolveKey(PlayerKey, BB_Player);
	const FName LKPKeyName = ResolveKey(LastKnownPositionKey, BB_LastKnownPosition);

	AActor* PlayerActor = Cast<AActor>(BB->GetValueAsObject(PlayerKeyName));

 	if (!IsValid(PlayerActor))
	{
		if (UWorld* World = OwnerComp.GetWorld())
		{
			if (World->GetNetMode() == NM_Standalone)
			{
				PlayerActor = UGameplayStatics::GetPlayerCharacter(World, 0);
				if (IsValid(PlayerActor))
				{
					BB->SetValueAsObject(PlayerKeyName, PlayerActor);
				}
			}
		}
	}

	if (IsValid(PlayerActor))
	{
		BB->SetValueAsVector(LKPKeyName, PlayerActor->GetActorLocation());
	}
}

void UBTService_ChasingBehavior::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		const FName InRangeKeyName = ResolveKey(PlayerWithinAttackRangeKey, BB_PlayerWithinRange);
		BB->SetValueAsBool(InRangeKeyName, false);
	}
}

FString UBTService_ChasingBehavior::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: '%s'\n%s: '%s'\n%s: '%s'\n%s: '%s'\n%s: '%s'"),
		TEXT("Player Class"), PlayerClass ? *PlayerClass->GetName() : TEXT("None"),
		TEXT("PlayerKey"), PlayerKey.IsSet() ? *PlayerKey.SelectedKeyName.ToString() : TEXT("None"),
		TEXT("LastKnownPositionKey"), LastKnownPositionKey.IsSet() ? *LastKnownPositionKey.SelectedKeyName.ToString() : TEXT("None"),
		TEXT("CanSeePlayerKey"), CanSeePlayerKey.IsSet() ? *CanSeePlayerKey.SelectedKeyName.ToString() : TEXT("None"),
		TEXT("CanHearPlayerKey"), CanHearPlayerKey.IsSet() ? *CanHearPlayerKey.SelectedKeyName.ToString() : TEXT("None"));
}