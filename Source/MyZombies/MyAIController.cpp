// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAIController.h"
#include "EnemyCharacter.h"
#include "MainCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"

const FName AMyAIController::KEY_CanSeePlayer(TEXT("CanSeePlayer"));
const FName AMyAIController::KEY_CanHearPlayer(TEXT("CanHearPlayer"));
const FName AMyAIController::KEY_Player(TEXT("Player"));
const FName AMyAIController::KEY_LastKnownPosition(TEXT("LastKnownPosition"));
const FName AMyAIController::KEY_PlayerWithinRange(TEXT("PlayerWithinRange"));

AMyAIController::AMyAIController()
{
    // Create AI Perception component as well as Sight configuration
    MyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	// Sight config (tune in editor/INI as needed)
	SightConfig->SightRadius = 600.f;
	SightConfig->LoseSightRadius = 700.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f; // keep default unless you know better

	// Hearing config
	HearingConfig->HearingRange = 1500.f;

	// Detect all affiliations (works without teams setup)
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;


    // Assign configured senses to the AI Perception Component
    MyPerceptionComponent->ConfigureSense(*SightConfig);
    MyPerceptionComponent->ConfigureSense(*HearingConfig);
    MyPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Binding the OnTargetPerceptionUpdate function
    SetPerceptionComponent(*MyPerceptionComponent);
    MyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMyAIController::OnTargetPerceptionUpdate);


}

/*
    Call parent version of OnPossess
    Create and initialize reference to main character and cast pawn to maincharacter 
    Check if pointer is not null
    Create and initialize behavior tree reference
    Check if behavior tree reference is not null
    Run behavior tree
*/
void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(InPawn);
	if (!Enemy) return;

	// Cache default speed to restore when not chasing.
	if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
	{
		DefaultWalkSpeed = Move->MaxWalkSpeed;
	}

	if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
	{
		if (BT->BlackboardAsset)
		{
			UBlackboardComponent* OutBB = nullptr;
			if (UseBlackboard(BT->BlackboardAsset, OutBB))
			{
				// Initialize BB to safe defaults (prevents stale state on level start).
				OutBB->SetValueAsBool(KEY_CanSeePlayer, false);
				OutBB->SetValueAsBool(KEY_CanHearPlayer, false);
				OutBB->SetValueAsBool(KEY_PlayerWithinRange, false);
				OutBB->SetValueAsVector(KEY_LastKnownPosition, FVector::ZeroVector);
				OutBB->SetValueAsObject(KEY_Player, nullptr);
				RunBehaviorTree(BT);
			}
		}
	}
}


void AMyAIController::OnTargetPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
    if (!BB || !Enemy) return;

    AMainCharacter* SeenPlayer = Cast<AMainCharacter>(Actor);
    if (!SeenPlayer) return;

    static const FAISenseID SightID = UAISense_Sight::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID();
    static const FAISenseID HearID = UAISense_Hearing::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID();

    bool bSee  = BB->GetValueAsBool(KEY_CanSeePlayer);
    bool bHear = BB->GetValueAsBool(KEY_CanHearPlayer);

    const bool bSensed = Stimulus.WasSuccessfullySensed();

    if (Stimulus.Type == SightID)
    {
        bSee = bSensed;
        if (bSensed)
        {
            BB->SetValueAsObject(KEY_Player, SeenPlayer);
            BB->SetValueAsVector(KEY_LastKnownPosition, Stimulus.StimulusLocation);
        }
    }
    else if (Stimulus.Type == HearID)
    {
        bHear = bSensed;
        if (bSensed)
        {
            BB->SetValueAsObject(KEY_Player, SeenPlayer);
            BB->SetValueAsVector(KEY_LastKnownPosition, Stimulus.StimulusLocation);
            Enemy->SetLastKnownPlayerPos(Stimulus.StimulusLocation);
        }
    }

    BB->SetValueAsBool(KEY_CanSeePlayer, bSee);
    BB->SetValueAsBool(KEY_CanHearPlayer, bHear);

    if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
    {
        const bool bChasing = (bSee || bHear);
        const float Desired = bChasing ? Enemy->GetChaseSpeed() : Enemy->GetWalkSpeed();
        SetWalkSpeedIfChanged(Move, Desired);
        Enemy->SetChasingState(bChasing);

    }

	AMainCharacter* BBPlayer = Cast<AMainCharacter>(BB->GetValueAsObject(KEY_Player));
	const bool bInRange = IsPlayerWithinRange(BBPlayer, Enemy);
	BB->SetValueAsBool(KEY_PlayerWithinRange, bInRange);

    Enemy->SetPerceptionState(bSee, bHear, bInRange);

    
       

}

void AMyAIController::UpdateNearbyAgents()
{
    // Logic to collect locations of nearby agents within a certain radius
    TArray<AActor*> DetectedAgents;
    MyPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), DetectedAgents);

    NearbyAgentLocations.Empty();
    for (AActor* Agent : DetectedAgents)
    {
        if (Agent != GetPawn()) // Skip the current AI's pawn
        {
            NearbyAgentLocations.Add(Agent->GetActorLocation());
        }
    }
}


bool AMyAIController::IsPlayerWithinRange(const AMainCharacter* Player, const AEnemyCharacter* Enemy) const
{
    if (!Player || !Enemy) return false;
    const float Range = Enemy->GetAttackRange(); // cm
    const float DistSq2D = FVector::DistSquared2D(Enemy->GetActorLocation(), Player->GetActorLocation());
    return DistSq2D <= FMath::Square(Range);
}


void AMyAIController::SetWalkSpeedIfChanged(UCharacterMovementComponent* MoveComp, float Desired)
{
	if (!MoveComp) return;
	if (!FMath::IsNearlyEqual(MoveComp->MaxWalkSpeed, Desired))
	{
		MoveComp->MaxWalkSpeed = Desired;
	}
}
