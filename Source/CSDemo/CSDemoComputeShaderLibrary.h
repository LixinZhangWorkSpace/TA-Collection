// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CSDemoComputeShaderLibrary.generated.h"

class UTextureRenderTarget2D;

UCLASS()
class CSDEMO_API UCSDemoComputeShaderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CSDemo|Compute Shader", meta = (DisplayName = "Fill Render Target Red (Compute Shader)"))
	static void FillRenderTargetRed(UTextureRenderTarget2D* OutputRenderTarget);

	UFUNCTION(BlueprintCallable, Category = "CSDemo|Compute Shader", meta = (DisplayName = "Run Compute Shader (Input RT -> Output RT)"))
	static void RunRedTestWithInput(UTextureRenderTarget2D* InputRenderTarget, UTextureRenderTarget2D* OutputRenderTarget);
};
