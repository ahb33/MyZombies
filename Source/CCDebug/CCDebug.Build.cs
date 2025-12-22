using UnrealBuildTool;

public class CCDebug : ModuleRules
{
    public CCDebug(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine"
            // add "UMG", "AIModule", etc. **only if CCDebug uses them**
        });
    }
}
