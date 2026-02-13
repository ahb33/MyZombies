#include "CCGameplayDebuggerCategory.h" 


#if WITH_GAMEPLAY_DEBUGGER 

#include "GameplayDebuggerTypes.h"
#include "InputCoreTypes.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
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

}


FCCGameplayDebuggerCategory::FCCGameplayDebuggerCategory()
{
    const FGameplayDebuggerInputHandlerConfig ToggleRangeCfg(TEXT("CC.ToggleRange"), EKeys::R.GetFName());
    BindKeyPress(ToggleRangeCfg, this, &FCCGameplayDebuggerCategory::ToggleRange, EGameplayDebuggerInputMode::Local);

    const FGameplayDebuggerInputHandlerConfig ToggleOnlyAICfg(TEXT("CC.ToggleOnlyAI"), EKeys::O.GetFName());
    BindKeyPress(ToggleOnlyAICfg, this, &FCCGameplayDebuggerCategory::ToggleOnlyAI, EGameplayDebuggerInputMode::Local);
}

void FCCGameplayDebuggerCategory::ToggleRange()
{
	bDrawRange = !bDrawRange;
}

void FCCGameplayDebuggerCategory::ToggleOnlyAI()
{
	bOnlyAI = !bOnlyAI;
}

void FCCGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	AddTextLine(TEXT("{green}[R]{white} toggle range  {green}[O]{white} toggle AI-only"));

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

    APawn* Pawn = Cast<APawn>(DebugActor);
    AAIController* AI = Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;

    if (bOnlyAI && !AI)
	{
		AddTextLine(TEXT("Selected actor is not AI-controlled. Press [O] to show anyway."));
		return;
	}


    UBlackboardComponent* BB = AI ? AI->GetBlackboardComponent() : nullptr;

    const bool bHasBB = (BB != nullptr);
    const bool bInRange = bHasBB ? BB->GetValueAsBool(TEXT("PlayerWithinRange")) : false;
    const bool bCanSee  = bHasBB ? BB->GetValueAsBool(TEXT("CanSeePlayer")) : false;
    const bool bCanHear = bHasBB ? BB->GetValueAsBool(TEXT("CanHearPlayer")) : false;

    const FVector Center = DebugActor->GetActorLocation();

    float Range = 0.f;
    const bool bHasRange = 
    TryReadFloat(DebugActor, TEXT("AttackRange"), Range) ||
    TryReadFloat(DebugActor, TEXT("InteractRange"), Range) ||
    TryReadFloat(DebugActor, TEXT("InteractionRange"), Range);

    const float SphereRadius = bHasRange ? Range : FMath::Max(150.f, GetCapsuleRadiusSafe(DebugActor) * 2.5f);    
    const FColor SphereColor  = bInRange ? FColor::Red : FColor::Green;

    AddTextLine(FString::Printf(TEXT("Actor: %s Class: %s"), *GetNameSafe(DebugActor), *GetNameSafe(DebugActor->GetClass()))); 
    
    AddTextLine(FString::Printf(TEXT("HasBB:%d CanSee:%d CanHear:%d InRange:%d"), bHasBB?1:0, bCanSee?1:0, bCanHear?1:0, bInRange?1:0)); 
    
    AddTextLine(FString::Printf(TEXT("RangeSphere SphereRadius: %.0f (source: %s)"), SphereRadius, bHasRange ? TEXT("prop") : TEXT("fallback"))); 
    
    AddShape(FGameplayDebuggerShape::MakeCapsule(Center, SphereRadius, SphereRadius, SphereColor, TEXT("Range")));
}

void FCCGameplayDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
    FGameplayDebuggerCategory::DrawData(OwnerPC, CanvasContext);
}

#endif 