#include "CCGameplayDebuggerCategory.h" 


#if WITH_GAMEPLAY_DEBUGGER 

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameplayDebuggerTypes.h"
#include "InputCoreTypes.h"
#include "CCDebug.h"
#include "UObject/UnrealType.h"


namespace
{
    static const FName BB_InRange(TEXT("PlayerWithinRange"));
    static const FName BB_CanSee(TEXT("CanSeePlayer"));
    static const FName BB_CanHear(TEXT("CanHearPlayer"));

    static bool TryReadFloat(const UObject* Obj, const FName PropName, float& OutValue)
    {
        if(!Obj) return false;

        static TMap<UClass*, TMap<FName, const FProperty*>> Cache;
        UClass* Cls = Obj->GetClass();
        if(!Cls) return false;

        const FProperty* const* Found = Cache.FindOrAdd(Cls).Find(PropName);

        const FProperty* Prop = Found ? *Found : nullptr;
        if (!Prop)
        {
            Prop = Cls->FindPropertyByName(PropName);
            Cache.FindOrAdd(Cls).Add(PropName, Prop); // caches null too
        }

        if (!Prop) return false;

        if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
        {
            OutValue = FloatProp->GetPropertyValue_InContainer(Obj);
            return true;
        }
        
        if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
        {
            OutValue = (float)DoubleProp->GetPropertyValue_InContainer(Obj);
            return true;
        }
        return false;

    }

    static float GetCapsuleRadiusSafe(AActor* Actor)
    {
        if(!Actor) return 0.f;
        if(UCapsuleComponent* Cap = Actor->FindComponentByClass<UCapsuleComponent>())
        {
            return Cap->GetScaledCapsuleRadius();
        }
        return 0.f;
    }

    static bool GetCapsuleDimsSafe(AActor* Actor, float& OutRadius, float& OutHalfHeight)
    {
        OutRadius = 0.f;
        OutHalfHeight = 0.f;
        if (!Actor) return false;

        if (UCapsuleComponent* Cap = Actor->FindComponentByClass<UCapsuleComponent>())
        {
            OutRadius     = Cap->GetScaledCapsuleRadius();
            OutHalfHeight = Cap->GetScaledCapsuleHalfHeight();
            return true;
        }
        return false;
    }

    static bool GetBBBoolSafe(const UBlackboardComponent* BB, const FName Key, bool& OutValue)
    {
        if (!BB || Key.IsNone()) return false;
        const FBlackboard::FKey KeyID = BB->GetKeyID(Key);
        if (KeyID == FBlackboard::InvalidKey) return false;

        OutValue = BB->GetValueAsBool(Key);
        return true;
    }
}


FCCGameplayDebuggerCategory::FCCGameplayDebuggerCategory()
{
	BindKeyPress(
		FGameplayDebuggerInputHandlerConfig(TEXT("CC.ToggleRange"), EKeys::R.GetFName()),
		this,
		&FCCGameplayDebuggerCategory::ToggleRange,
		EGameplayDebuggerInputMode::Local
	);

	BindKeyPress(
		FGameplayDebuggerInputHandlerConfig(TEXT("CC.ToggleOnlyAI"), EKeys::O.GetFName()),
		this,
		&FCCGameplayDebuggerCategory::ToggleOnlyAI,
		EGameplayDebuggerInputMode::Local
	);

	BindKeyPress(
		FGameplayDebuggerInputHandlerConfig(TEXT("CC.ToggleVision"), EKeys::V.GetFName()),
		this,
		&FCCGameplayDebuggerCategory::ToggleVision,
		EGameplayDebuggerInputMode::Local
	);
}

void FCCGameplayDebuggerCategory::ToggleRange()
{
	bDrawRange = !bDrawRange;
}

void FCCGameplayDebuggerCategory::ToggleOnlyAI()
{
	bOnlyAI = !bOnlyAI;
}

void FCCGameplayDebuggerCategory::ToggleVision()
{
    bDrawVision = !bDrawVision;
}

void FCCGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	AddTextLine(TEXT("{green}[R]{white} toggle range  {green}[O]{white} toggle AI-only {green}[V]{white} vision "));

    if (!OwnerPC)
    {
        AddTextLine(TEXT("No OwnerPC"));
        return;
    }

    if (!DebugActor)
    {
        AddTextLine(TEXT("No DebugActor selected."));
        return;
    }

    AddTextLine(bOnlyAI ? TEXT("AI-only: {green}ON") : TEXT("AI-only: {red}OFF"));

	UWorld* World = DebugActor->GetWorld();
	AddTextLine(FString::Printf(TEXT("NetMode: %s  Role: %s"),
		CCD_NetModeStr(World),
		CCD_RoleStr(DebugActor)));

	AddTextLine(FString::Printf(TEXT("Actor: %s  Class: %s"),
		*GetNameSafe(DebugActor),
		*GetNameSafe(DebugActor->GetClass())));

    APawn* Pawn = Cast<APawn>(DebugActor);
    if (!Pawn)
    {
        AddTextLine(TEXT("{red}Select an AI Pawn/Character (not a Controller)."));
        return;
    }

    AAIController* AI = Cast<AAIController>(Pawn->GetController());
    if (bOnlyAI && !AI)
    {
        AddTextLine(TEXT("{yellow}Not AI-controlled. Press [O] to show anyway."));
        return;
    }
       
	if (World && World->GetNetMode() == NM_Client && AI)
	{
		AddTextLine(TEXT("{red}WARNING: AIController present on Client. Verify authority-only AI logic."));
	}
   
	AddTextLine(FString::Printf(TEXT("Controller: %s"), AI ? *GetNameSafe(AI) : TEXT("<none>")));

    UBlackboardComponent* BB = AI ? AI->GetBlackboardComponent() : nullptr;

    bool bBBInRange = false, bBBCanSee = false, bBBCanHear = false;
    const bool bHasInRange = GetBBBoolSafe(BB, BB_InRange, bBBInRange);
    const bool bHasCanSee  = GetBBBoolSafe(BB, BB_CanSee,  bBBCanSee);
    const bool bHasCanHear = GetBBBoolSafe(BB, BB_CanHear, bBBCanHear);

	AddTextLine(FString::Printf(
		TEXT("BB: %s=%s  %s=%s  %s=%s"),
		*BB_InRange.ToString(), bHasInRange ? (bBBInRange ? TEXT("true") : TEXT("false")) : TEXT("<missing>"),
		*BB_CanSee.ToString(),  bHasCanSee  ? (bBBCanSee  ? TEXT("true") : TEXT("false")) : TEXT("<missing>"),
		*BB_CanHear.ToString(), bHasCanHear ? (bBBCanHear ? TEXT("true") : TEXT("false")) : TEXT("<missing>")
	));

    // Player distance + range
    FVector Center = Pawn->GetActorLocation();
    if (const UCapsuleComponent* Cap = Pawn->FindComponentByClass<UCapsuleComponent>())
    {
        Center = Cap->GetComponentLocation();
    }
    AActor* Target = DebugActor;

    float Range = 0.f;

    const bool bHasRange = 
    TryReadFloat(DebugActor, TEXT("AttackRange"), Range) ||
    TryReadFloat(DebugActor, TEXT("InteractRange"), Range) ||
    TryReadFloat(DebugActor, TEXT("InteractionRange"), Range);

    const float Fallback  = FMath::Max(150.f, GetCapsuleRadiusSafe(DebugActor) * 2.5f);
    const float Radius = bHasRange ? Range : Fallback;

    APawn* PlayerPawn = OwnerPC->GetPawn();
    const float PlayerDist = PlayerPawn ? FVector::Dist(Center, PlayerPawn->GetActorLocation()) : TNumericLimits<float>::Max();
    const bool bComputedInRange = PlayerPawn && (PlayerDist <= Radius);

    AddTextLine(FString::Printf(TEXT("PlayerDist: %.1f  Radius: %.1f  InRange: %s"),
		PlayerDist, Radius, bComputedInRange ? TEXT("true") : TEXT("false")));

	if (bDrawRange)
	{
        FVector RangeCenter = DebugActor->GetActorLocation();

        // Use capsule center if present (most characters/pawns).
        if (const UCapsuleComponent* Cap = DebugActor->FindComponentByClass<UCapsuleComponent>())
        {
            RangeCenter = Cap->GetComponentLocation();
        }
        // If the debug actor is an AIController, draw around its pawn instead.
        else if (const AAIController* AIC = Cast<AAIController>(DebugActor))
        {
            if (const APawn* P = AIC->GetPawn())
            {
                RangeCenter = P->GetActorLocation();
                if (const UCapsuleComponent* PawnCap = P->FindComponentByClass<UCapsuleComponent>())
                {
                    RangeCenter = PawnCap->GetComponentLocation();
                }
            }
        }

        const FColor RangeColor = bComputedInRange ? FColor::Green : FColor::Red;

        // Sphere-ish bubble (capsule degenerated to sphere when HalfHeight == Radius).
        AddShape(FGameplayDebuggerShape::MakeCapsule(RangeCenter,
            Radius,   // HalfHeight
            Radius,   // Radius
            RangeColor,
            TEXT("Range")
        ));
    }

	if (bDrawVision && PlayerPawn)
	{
		const bool bLOS = (AI != nullptr) ? AI->LineOfSightTo(PlayerPawn) : false;
		const bool bSees = bBBCanSee || bLOS;

		const FColor VisionColor = bSees ? FColor::Cyan : FColor(128, 128, 128);
		AddShape(FGameplayDebuggerShape::MakeSegment(Center, PlayerPawn->GetActorLocation(), VisionColor, TEXT("Vision")));
		AddTextLine(FString::Printf(TEXT("Vision: BB=%s  LOS=%s"),
			bBBCanSee ? TEXT("true") : TEXT("false"),
			bLOS ? TEXT("true") : TEXT("false")));
	}
}

void FCCGameplayDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
    FGameplayDebuggerCategory::DrawData(OwnerPC, CanvasContext);
}

#endif 