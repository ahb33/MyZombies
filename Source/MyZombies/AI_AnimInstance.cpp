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
        if (PawnInstance)
        {        
            EnemyCharacterInstance = Cast<AEnemyCharacter>(PawnInstance);
        }

        /* if both checks above succeed, we will have a variable with access to the pawn in
        the forms of APawn and AMainCharacter */
    }
}

void UAI_AnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    // Ensure that we always have a valid reference to Pawn.
    if (PawnInstance == nullptr)
    {
        PawnInstance = TryGetPawnOwner();
        // Attempt to cast once and cache the result for future frames.
        EnemyCharacterInstance = Cast<AEnemyCharacter>(PawnInstance);
    }

    // Once Pawn is valid, we proceed to update the animation properties.
    if (PawnInstance)
    {
        const FVector Vel = PawnInstance->GetVelocity();
        const float TargetSpeed = FVector(Vel.X, Vel.Y, 0.f).Size();

        const UCharacterMovementComponent* Move = EnemyCharacterInstance ? EnemyCharacterInstance->GetCharacterMovement() : nullptr;
        bIsAccelerating = Move && Move->GetCurrentAcceleration().Size() > 0.f;

        MovementSpeed = FMath::FInterpTo(MovementSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);

        // Clamp to CURRENT MaxWalkSpeed (dynamic), not a fixed 360
        const float CurrentMax = Move ? Move->MaxWalkSpeed : 360.f;
        MovementSpeed = FMath::Clamp(MovementSpeed, 0.f, CurrentMax);

        UE_LOG(LogTemp, VeryVerbose, TEXT("[Anim] TargetSpeed=%.1f MovementSpeed=%.1f MaxWalkSpeed=%.1f"),
            TargetSpeed, MovementSpeed, CurrentMax);

        if (AAIController* AIC = Cast<AAIController>(PawnInstance->GetController()))
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            SetPlayerVisibility(BB->GetValueAsBool(TEXT("CanSeePlayer")));
            SetPlayerAttackRange(BB->GetValueAsBool(TEXT("PlayerWithinAttackRange")));
        }
    }
}
