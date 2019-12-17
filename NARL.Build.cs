// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class NARL : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    public NARL(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "ProceduralMeshComponent"
        });

		PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "ImageWrapper",
            "RenderCore"
        });

        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ThirdPartyPath, "TinyObjLoader/")
        });

        PublicDefinitions.Add("TINYOBJLOADER_IMPLEMENTATION");

    }
}
