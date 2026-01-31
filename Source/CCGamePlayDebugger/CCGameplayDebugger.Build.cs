using UnrealBuildTool;

public class CCGameplayDebugger : ModuleRules 
{ 
    public CCGameplayDebugger(ReadOnlyTargetRules Target) 
    :   base(Target) 
    { 
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs; 

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine" 
        }); 

        PrivateDependencyModuleNames.AddRange(new string[]     
        { 
            "GameplayTags" // for IGameplayTagAssetInterface / GameplayTagContainer 
        }); 

        // GameplayDebugger is developer tooling; best kept out of Shipping. 
        if (Target.bBuildDeveloperTools) 
        { 
            PrivateDependencyModuleNames.Add("GameplayDebugger"); 
        }         
    }
}

