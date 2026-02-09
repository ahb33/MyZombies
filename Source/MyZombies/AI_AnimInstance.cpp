// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "EnemyCharacter.h"
#include "BTService_ChasingBehavior.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAIController.h"



void UAI_AnimInstance::NativeInitializeAnimation()
{

    Super::NativeInitializeAnimation(); // this calls the parent function to ensure code in it gets called
    if(PawnInstance == nullptr) // the following code is to ensure we the function does not return a nullptr which could cause a crash
    {               
        
        PawnInstance = TryGetPawnOwner(); // this function will return the Pawn belonging to this animinstance
        EnemyCharacterInstance = Cast<AEnemyCharacter>(PawnInstance);

        if (PawnInstance)
        {
            PrevLoc = PawnInstance->GetActorLocation();
            bHasPrevLoc = true;
        }

        /* if both checks above succeed, we will have a variable with access to the pawn in
        the forms of APawn and AMainCharacter */
    }
}

void UAI_AnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!PawnInstance)
    {
        PawnInstance = TryGetPawnOwner();
        EnemyCharacterInstance = Cast<AEnemyCharacter>(PawnInstance);
        if (PawnInstance)
        {
            PrevLoc = PawnInstance->GetActorLocation();
            bHasPrevLoc = true;
        }
    }

    if (!PawnInstance) return;

    // Speed from velocity (if valid) OR from location delta (robust)
    const FVector Vel = PawnInstance->GetVelocity();
    const float VelSpeed2D = FVector(Vel.X, Vel.Y, 0.f).Size();

    float LocSpeed2D = 0.f;
    const FVector Loc = PawnInstance->GetActorLocation();
    if (bHasPrevLoc && DeltaTime > KINDA_SMALL_NUMBER)
    {
        LocSpeed2D = FVector::Dist2D(Loc, PrevLoc) / DeltaTime;
    }
    PrevLoc = Loc;
    bHasPrevLoc = true;

    const float TargetSpeed = FMath::Max(VelSpeed2D, LocSpeed2D);

    MovementSpeed = FMath::FInterpTo(MovementSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);

    if (EnemyCharacterInstance)
    {
        SetPlayerVisibility(EnemyCharacterInstance->CanSeePlayer());
        SetPlayerAttackRange(EnemyCharacterInstance->IsPlayerWithinRange());
    }
}
