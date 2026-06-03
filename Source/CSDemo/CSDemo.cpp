// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSDemo.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "ShaderCore.h"

void FCSDemoModule::StartupModule()
{
	const FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	const FString VirtualShaderDirectory = TEXT("/CSDemo");
	const TMap<FString, FString>& ShaderMappings = AllShaderSourceDirectoryMappings();

	if (const FString* ExistingMapping = ShaderMappings.Find(VirtualShaderDirectory))
	{
		if (*ExistingMapping != ShaderDirectory)
		{
			UE_LOG(LogTemp, Warning, TEXT("Shader directory mapping for %s already exists and points to %s."), *VirtualShaderDirectory, **ExistingMapping);
		}
		return;
	}

	AddShaderSourceDirectoryMapping(VirtualShaderDirectory, ShaderDirectory);
}

void FCSDemoModule::ShutdownModule()
{
}

IMPLEMENT_PRIMARY_GAME_MODULE(FCSDemoModule, CSDemo, "CSDemo");
