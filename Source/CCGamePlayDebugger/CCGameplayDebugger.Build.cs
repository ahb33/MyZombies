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
			"CCDebug",
			"Engine",
			"GameplayDebugger",
			"AIModule",
			"InputCore"
		});
    }
}

