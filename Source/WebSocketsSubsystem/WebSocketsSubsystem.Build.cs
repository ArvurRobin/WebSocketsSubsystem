// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

using UnrealBuildTool;

public class WebSocketsSubsystem : ModuleRules
{
    public WebSocketsSubsystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "WebSockets"
        });
    }
}
