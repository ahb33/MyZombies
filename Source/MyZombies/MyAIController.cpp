// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAIController.h"
#include "BTService_ChasingBehavior.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyCharacter.h"





AMyAIController::AMyAIController()
{
    // Create AI Perception component as well as Sight configuration
    MyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("MyPerceptionComponent"));

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

    // Configuring the Sight Sense 
    SightConfig->SightRadius = 600.0f;
    SightConfig->LoseSightRadius = 700.0f;
    HearingConfig->HearingRange = 1500.0f;

    // Check all the flags for DetectionByAffiliation so that we detect our Player
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // Assign configured senses to the AI Perception Component
    MyPerceptionComponent->ConfigureSense(*SightConfig);
    MyPerceptionComponent->ConfigureSense(*HearingConfig);

    
    MyPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Binding the OnTargetPerceptionUpdate function
    MyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMyAIController::OnTargetPerceptionUpdate);

    UE_LOG(LogTemp, Warning, TEXT("Hearing Config Range: %f"), HearingConfig->HearingRange);

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
    AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(InPawn);

    if (EnemyCharacter != nullptr)
    {
        UBehaviorTree* BehaviorTree = EnemyCharacter->BehaviorTree;
        if (BehaviorTree != nullptr)
        {
            RunBehaviorTree(BehaviorTree);
        }
        
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]() { UpdateNearbyAgents(); });
    }
}

void AMyAIController::OnTargetPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{

    if (Stimulus.Type == UAISense_Hearing::StaticClass()->GetDefaultObject<UAISense>()->GetSenseID())
    {
        // UE_LOG(LogTemp, Warning, TEXT("Hearing Stimulus detected."));
    }
    // Retrieve all currently perceived Actors - OnTargetPerceptionUpdate requires array of actors 
    // that it can fill
    
    // Retrieve all currently perceived Actors for sight
    TArray<AActor*> PerceivedActorsSight;
    PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActorsSight);

    // Retrieve all currently perceived Actors for hearing
    TArray<AActor*> PerceivedActorsHearing;
    PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Hearing::StaticClass(), PerceivedActorsHearing);

    // Combine unique actors from both lists
    TArray<AActor*> AllPerceivedActors = PerceivedActorsSight;
    for (AActor* HeardActor : PerceivedActorsHearing)
    {
        AllPerceivedActors.AddUnique(HeardActor);
    }

    // Retrieve Blackboard Component
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Blackboard Component is nullptr."));
        return;
    }    
    
    // Cast the Actor to AMainCharacter
    MainCharacter = Cast<AMainCharacter>(Actor);
    if (MainCharacter && AllPerceivedActors.Contains(MainCharacter))
    {
        LastKnownPlayerPosition = Stimulus.StimulusLocation;

        // test this approach if all else fails
        // bool bCanSeePlayer = Stimulus.WasSuccessfullySensed();
        bool bCanSeePlayer = PerceivedActorsSight.Contains(MainCharacter);
        bool bCanHearPlayer = PerceivedActorsHearing.Contains(MainCharacter);
        // UE_LOG(LogTemp, Warning, TEXT("Updated Blackboard with CanSeePlayer: %s, CanHearPlayer: %s"),
        // bCanSeePlayer ? TEXT("true") : TEXT("false"),
        // bCanHearPlayer ? TEXT("true") : TEXT("false"));

        BlackboardComp->SetValueAsBool("CanSeePlayer", bCanSeePlayer);
        BlackboardComp->SetValueAsBool("CanHearPlayer", bCanHearPlayer);
        // UE_LOG(LogTemp, Warning, TEXT("Blackboard Comp is valid"));

        if (bCanSeePlayer || bCanHearPlayer)
        {
            BlackboardComp->SetValueAsVector("LastKnownPosition", LastKnownPlayerPosition);
        }
    }

    else
    {
        BlackboardComp->SetValueAsBool("CanSeePlayer", false);
        BlackboardComp->SetValueAsBool("CanHearPlayer", false);        
    }

    
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