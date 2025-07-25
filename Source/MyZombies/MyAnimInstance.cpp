// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "GameFramework/Character.h"
#include "Misc/App.h"
#include "Kismet/KismetMathLibrary.h"


void UMyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        MainCharacter = Cast<AMainCharacter>(Pawn);
        if (MainCharacter)
        {
            playerWeapon = MainCharacter->GetEquippedWeapon();
        }
    }
}
void UMyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!MainCharacter)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            MainCharacter = Cast<AMainCharacter>(Pawn);
        }
        if (!MainCharacter) return;
    }

    // Get the equipped weapon
    playerWeapon = MainCharacter->GetEquippedWeapon();
    
    // Continue updates if MainCharacter is valid
    UpdateMovementProperties();
    UpdateCharacterProperties(DeltaTime);
    UpdateWeaponProperties();

}

void UMyAnimInstance::UpdateMovementProperties()
{
    FVector Speed = Pawn->GetVelocity();
    FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
    MovementSpeed = LateralSpeed.Size();

    bIsInAir = Pawn->GetMovementComponent()->IsFalling();
    bIsAccelerating = MainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0;
}

void UMyAnimInstance::UpdateCharacterProperties(float DeltaTime)
{
    bWeaponEquipped = MainCharacter->IsWeaponEquipped();
    playerWeapon = MainCharacter->GetEquippedWeapon();
    bIsCrouched = MainCharacter->bIsCrouched;
    bAiming = MainCharacter->IsAiming();
    TurningInPlace = MainCharacter->GetTurningInPlace();

    FRotator AimRotation = MainCharacter->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MainCharacter->GetVelocity());
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);
    YawOffset = DeltaRotation.Yaw;

    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = MainCharacter->GetActorRotation();
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    float Target = Delta.Yaw / DeltaTime;
    float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
    Lean = FMath::Clamp(Interp, -90.f, 90.f);

    AO_Yaw = MainCharacter->GetCharacterYaw();
    AO_Pitch = MainCharacter->GetCharacterPitch();
}

void UMyAnimInstance::UpdateWeaponProperties()
{
    if (bWeaponEquipped && playerWeapon && playerWeapon->GetWeaponMesh() && MainCharacter->GetMesh())
    {
        LeftHandTransform = playerWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
        
        FVector OutPosition;
        FRotator OutRotation;
        MainCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(FQuat(OutRotation));

        if (MainCharacter->IsLocallyControlled())
        {
            bLocallyControlled = true;
            FTransform RightHandTransform = playerWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
            FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MainCharacter->GetHitTarget()));

            bUseFABRIK = MainCharacter->GetCharacterCombatState() != ECombatState::ECS_Reloading;

            if (bIsPlayerReloading)
            {
                UE_LOG(LogTemp, Warning, TEXT("bIsPlayerReloading is valid within AnimInstance"));
            }
        }
    }
}