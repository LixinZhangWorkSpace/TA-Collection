// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

class FCSDemoRedFillCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCSDemoRedFillCS);
	SHADER_USE_PARAMETER_STRUCT(FCSDemoRedFillCS, FGlobalShader);

public:
	static constexpr int32 ThreadGroupSizeX = 8;
	static constexpr int32 ThreadGroupSizeY = 8;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
		SHADER_PARAMETER(FIntPoint, InputTextureSize)
		SHADER_PARAMETER(FIntPoint, TextureSize)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static FIntVector GetGroupCount(FIntPoint TextureSize);
};
