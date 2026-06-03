// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSDemoRedFillCS.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "RenderGraphUtils.h"

bool FCSDemoRedFillCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
}

void FCSDemoRedFillCS::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), ThreadGroupSizeX);
	OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), ThreadGroupSizeY);
}

FIntVector FCSDemoRedFillCS::GetGroupCount(FIntPoint TextureSize)
{
	return FComputeShaderUtils::GetGroupCount(TextureSize, FIntPoint(ThreadGroupSizeX, ThreadGroupSizeY));
}

IMPLEMENT_GLOBAL_SHADER(FCSDemoRedFillCS, "/CSDemo/Private/CSDemoRedFillCS.usf", "MainCS", SF_Compute);
