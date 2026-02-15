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

        if(const FProperty* Prop = Obj->GetClass()->FindPropertyByName(PropName))
        {
            if(const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
            {
                OutValue = FloatProp->GetPropertyValue_InContainer(Obj);
                return true;
            }
            if(const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
            {
                OutValue = static_cast<float>(DoubleProp->GetPropertyValue_InContainer(Obj));
                return true;
            }
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

    static bool GetBBBoolSafe(const UBlackboardComponent* BB, const FName Key, bool& OutValue)
    {
        if (!BB || Key.IsNone()) return false;
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

    
	UWorld* World = DebugActor->GetWorld();
	AddTextLine(FString::Printf(TEXT("NetMode: %s  Role: %s"),
		CCD_NetModeStr(World),
		CCD_RoleStr(DebugActor)));

	AddTextLine(FString::Printf(TEXT("Actor: %s  Class: %s"),
		*GetNameSafe(DebugActor),
		*GetNameSafe(DebugActor->GetClass())));

    APawn* Pawn = Cast<APawn>(DebugActor);
    AAIController * AI = Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;
    
    if (bOnlyAI && !AI)
	{
		AddTextLine(TEXT("{yellow} Not AI-controlled. Press [O] to show anyway."));
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
    const FVector Center = DebugActor->GetActorLocation();

    float Range = 0.f;
    FString RangeSource = TEXT("Fallback");

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
		const FColor RangeColor = bComputedInRange ? FColor::Green : FColor::Red;
		AddShape(FGameplayDebuggerShape::MakeCapsule(Center, Radius, Radius, RangeColor, TEXT("Range")));
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