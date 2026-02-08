#include "CCGameplayDebuggerCategory.h" 

#if WITH_GAMEPLAY_DEBUGGER 

#include "GameFramework/Actor.h" 
#include "GameFramework/PlayerController.h" 
#include "GameplayTagAssetInterface.h" 
#include "Components/CapsuleComponent.h"
#include "Engine/ActorChannel.h" 
#include "GameplayTagContainer.h" 
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "UObject/UnrealType.h" 

static const TCHAR* RoleToText(ENetRole Role) 
{ 
    switch (Role) 
    { case ROLE_Authority: return TEXT("Authority"); 
        case ROLE_AutonomousProxy: return TEXT("AutoProxy"); 
        case ROLE_SimulatedProxy: return TEXT("SimProxy"); 
        default: 
        return TEXT("Unknown"); 
    } 
} 
static bool TryReadBool(const UObject* Obj, const FName PropName, bool& OutValue)
{
    if (!Obj) return false;

    if (const FProperty* Prop = Obj->GetClass()->FindPropertyByName(PropName))
    {
        if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            OutValue = BoolProp->GetPropertyValue_InContainer(Obj);
            return true;
        }
    }
    return false;
}

static bool TryReadFloat(const UObject* Obj, const FName PropName, float& OutValue)
{
    if (!Obj) return false;

    if (const FProperty* Prop = Obj->GetClass()->FindPropertyByName(PropName))
    {
        if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
        {
            OutValue = FloatProp->GetPropertyValue_InContainer(Obj);
            return true;
        }

        if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
        {
            OutValue = static_cast<float>(DoubleProp->GetPropertyValue_InContainer(Obj));
            return true;
        }
    }
    return false;
}
 
namespace
{
	static const FName BB_CanSee(TEXT("CanSeePlayer"));
	static const FName BB_CanHear(TEXT("CanHearPlayer"));
	static const FName BB_Player(TEXT("Player"));
	static const FName BB_InRange(TEXT("PlayerWithinRange"));
	static const FName BB_LKP(TEXT("LastKnownPosition"));

	static float GetCapsuleRadiusSafe(AActor* A)
	{
        if (!A) return 0.f;

        if (UCapsuleComponent* Cap = A->FindComponentByClass<UCapsuleComponent>())
        {
            return Cap->GetScaledCapsuleRadius();
        }
		return 0.f;
	}

	static void AddBBLine(TArray<FString>& OutLines, const TCHAR* Label, bool bValue, bool bValid)
	{
		OutLines.Add(FString::Printf(TEXT("%s: %s%s"),
			Label,
			bValue ? TEXT("true") : TEXT("false"),
			bValid ? TEXT("") : TEXT(" (missing key)")));
	}
}

void FCCGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
    Lines.Reset();

    bDrawSphere = false;
    if (!OwnerPC)
    {
        Lines.Add(TEXT("No OwnerPC"));
        return;
    }

    if (!DebugActor)
    {
        Lines.Add(TEXT("No DebugActor selected (hover actor or lock selection)."));
        return;
    }

    APawn* Pawn = Cast<APawn>(DebugActor);
    AAIController* AI = Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;
    UBlackboardComponent* BB = AI ? AI->GetBlackboardComponent() : nullptr;

    const bool bHasBB = (BB != nullptr);

    const bool bInRange = bHasBB ? BB->GetValueAsBool(TEXT("PlayerWithinRange")) : false;
    const bool bCanSee  = bHasBB ? BB->GetValueAsBool(TEXT("CanSeePlayer")) : false;
    const bool bCanHear = bHasBB ? BB->GetValueAsBool(TEXT("CanHearPlayer")) : false;

    SphereCenter = DebugActor->GetActorLocation();

    float Range = 0.f;
    const bool bHasRange = 
    TryReadFloat(DebugActor, TEXT("AttackRange"), Range) ||
    TryReadFloat(DebugActor, TEXT("InteractRange"), Range) ||
    TryReadFloat(DebugActor, TEXT("InteractionRange"), Range);

    SphereRadius = bHasRange ? Range : FMath::Max(150.f, GetCapsuleRadiusSafe(DebugActor) * 2.5f);
    SphereColor  = bInRange ? FColor::Red : FColor::Green;
    bDrawSphere  = true;

    Lines.Add(FString::Printf(TEXT("Actor: %s  Class: %s"),
    *GetNameSafe(DebugActor),
    *GetNameSafe(DebugActor->GetClass())));

    Lines.Add(FString::Printf(TEXT("HasBB: %d  CanSee: %d  CanHear: %d  InRange: %d"),
    bHasBB ? 1 : 0,
    bCanSee ? 1 : 0,
    bCanHear ? 1 : 0,
    bInRange ? 1 : 0));

    Lines.Add(FString::Printf(TEXT("RangeSphere Radius: %.0f (source: %s)"),
    SphereRadius,
    bHasRange ? TEXT("prop") : TEXT("fallback")));
}

void FCCGameplayDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
    for (const FString& L : Lines)
    {
        CanvasContext.Printf(TEXT("%s"), *L);
    }

    if (!bDrawSphere || !OwnerPC) return;

    UWorld* World = OwnerPC->GetWorld();
    if (!World) return;

    // Lifetime 0 => redraw each frame while debugger is open/active.
    DrawDebugSphere(World, SphereCenter, SphereRadius, 24, SphereColor, false, 0.f, 0, 2.0f);
}

#endif 