#include "CCGameplayDebuggerCategory.h" 

#if WITH_GAMEPLAY_DEBUGGER 

#include "GameFramework/Actor.h" 
#include "GameFramework/PlayerController.h" 
#include "GameplayTagAssetInterface.h" 
#include "Engine/ActorChannel.h" 
#include "GameplayTagContainer.h" 
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
 

void FCCGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
    Lines.Reset();

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

    const FVector Loc = DebugActor->GetActorLocation();
    const FVector Vel = DebugActor->GetVelocity();

    Lines.Add(FString::Printf(TEXT("Actor: %s  Class: %s"),
        *GetNameSafe(DebugActor),
        *GetNameSafe(DebugActor->GetClass())));

    // UE 5.4: GetNetDormancy(...) returns bool and requires view + viewer context.
    FVector ViewPos;
    FRotator ViewRot;
    OwnerPC->GetPlayerViewPoint(ViewPos, ViewRot);

    const FVector ViewDir = ViewRot.Vector();
    AActor* Viewer = OwnerPC->GetPawn();
    AActor* ViewTarget = OwnerPC->GetViewTarget();
    const float TimeSeconds = OwnerPC->GetWorld() ? OwnerPC->GetWorld()->TimeSeconds : 0.f;

    const bool bDormantForViewer = DebugActor->GetNetDormancy(
        ViewPos,
        ViewDir,
        Viewer,
        ViewTarget,
        /*InChannel=*/nullptr,
        TimeSeconds,
        /*bLowBandwidth=*/false
    );

    Lines.Add(FString::Printf(TEXT("Role: %s  DormantForMe: %d  NetUpdateFreq: %.1f"),
        RoleToText(DebugActor->GetLocalRole()),
        bDormantForViewer ? 1 : 0,
        DebugActor->NetUpdateFrequency));

    Lines.Add(FString::Printf(TEXT("Owner: %s"),
        *GetNameSafe(DebugActor->GetOwner())));

    Lines.Add(FString::Printf(TEXT("Location: (%.0f, %.0f, %.0f)  Speed: %.1f"),
        Loc.X, Loc.Y, Loc.Z, Vel.Size()));

    // Gameplay Tags (if any)
    if (const IGameplayTagAssetInterface* TagsIf = Cast<IGameplayTagAssetInterface>(DebugActor))
    {
        FGameplayTagContainer Tags;
        TagsIf->GetOwnedGameplayTags(Tags);

        if (Tags.Num() > 0)
        {
            Lines.Add(TEXT("Tags:"));
            for (const FGameplayTag& T : Tags)
            {
                Lines.Add(FString::Printf(TEXT("  - %s"), *T.ToString()));
            }
        }
    }

    // Generic health read (no dependency on your game module)
    float HP = 0.f, MaxHP = 0.f;

    const bool bHasHP =
        TryReadFloat(DebugActor, TEXT("PlayerHealth"), HP) ||
        TryReadFloat(DebugActor, TEXT("Health"), HP);

    const bool bHasMax =
        TryReadFloat(DebugActor, TEXT("MaxHealth"), MaxHP) ||
        TryReadFloat(DebugActor, TEXT("MaxHP"), MaxHP);

    if (bHasHP || bHasMax)
    {
        Lines.Add(FString::Printf(TEXT("Health: %.1f / %.1f"), HP, MaxHP));
    }

    bool bDead = false;
    if (TryReadBool(DebugActor, TEXT("bIsDead"), bDead))
    {
        Lines.Add(FString::Printf(TEXT("Dead: %s"), bDead ? TEXT("true") : TEXT("false")));
    }
}

void FCCGameplayDebuggerCategory::DrawData(APlayerController* /*OwnerPC*/, FGameplayDebuggerCanvasContext& CanvasContext)
{
    for (const FString& L : Lines)
    {
        CanvasContext.Printf(TEXT("%s"), *L);
    }
}

#endif 