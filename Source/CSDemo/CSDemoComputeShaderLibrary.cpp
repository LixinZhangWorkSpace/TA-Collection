// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSDemoComputeShaderLibrary.h"

#include "CSDemoRedFillCS.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "TextureResource.h"

namespace
{
	constexpr ETextureRenderTargetFormat RequiredOutputFormat = RTF_RGBA16f;
	constexpr EPixelFormat RequiredOutputPixelFormat = PF_FloatRGBA;

	bool PrepareOutputRenderTarget(UTextureRenderTarget2D* OutputRenderTarget)
	{
		if (OutputRenderTarget == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Compute shader call received a null output render target."));
			return false;
		}

		const uint32 SafeSizeX = FMath::Max(OutputRenderTarget->SizeX, 1);
		const uint32 SafeSizeY = FMath::Max(OutputRenderTarget->SizeY, 1);
		const bool bNeedsCompatibleFormat = OutputRenderTarget->GetFormat() != RequiredOutputPixelFormat;
		const bool bNeedsUAVFlag = !OutputRenderTarget->bCanCreateUAV;

		if (bNeedsCompatibleFormat || bNeedsUAVFlag)
		{
			OutputRenderTarget->RenderTargetFormat = RequiredOutputFormat;
			OutputRenderTarget->bCanCreateUAV = true;
			OutputRenderTarget->InitCustomFormat(SafeSizeX, SafeSizeY, RequiredOutputPixelFormat, true);
			OutputRenderTarget->UpdateResourceImmediate(false);

			UE_LOG(
				LogTemp,
				Log,
				TEXT("Prepared output render target %s for compute shader use. Size=%ux%u Format=PF_FloatRGBA UAV=%d"),
				*OutputRenderTarget->GetName(),
				SafeSizeX,
				SafeSizeY,
				OutputRenderTarget->bCanCreateUAV ? 1 : 0);
		}

		return true;
	}

	FTextureRenderTargetResource* GetRenderTargetResourceChecked(UTextureRenderTarget2D* RenderTarget, const TCHAR* Label)
	{
		if (RenderTarget == nullptr)
		{
			return nullptr;
		}

		FTextureRenderTargetResource* Resource = RenderTarget->GameThread_GetRenderTargetResource();
		if (Resource == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s could not get a render target resource for %s."), Label, *RenderTarget->GetName());
		}

		return Resource;
	}
}

void UCSDemoComputeShaderLibrary::FillRenderTargetRed(UTextureRenderTarget2D* OutputRenderTarget)
{
	if (!PrepareOutputRenderTarget(OutputRenderTarget))
	{
		return;
	}

	FTextureRenderTargetResource* OutputRenderTargetResource = GetRenderTargetResourceChecked(OutputRenderTarget, TEXT("FillRenderTargetRed"));
	if (OutputRenderTargetResource == nullptr)
	{
		return;
	}

	const FIntPoint TextureSize(OutputRenderTarget->SizeX, OutputRenderTarget->SizeY);
	if (TextureSize.X <= 0 || TextureSize.Y <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FillRenderTargetRed received an invalid render target size for %s."), *OutputRenderTarget->GetName());
		return;
	}

	ENQUEUE_RENDER_COMMAND(CSDemoFillRenderTargetRed)(
		[OutputRenderTargetResource, TextureSize](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList, RDG_EVENT_NAME("CSDemo.FillRenderTargetRed"));
			FTextureRenderTarget2DResource* OutputRenderTarget2DResource = OutputRenderTargetResource->GetTextureRenderTarget2DResource();
			if (OutputRenderTarget2DResource == nullptr || !OutputRenderTarget2DResource->GetUnorderedAccessViewRHI().IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("FillRenderTargetRed aborted because the output render target does not expose a valid UAV."));
				return;
			}

			FRDGTextureDesc DummyInputDesc = FRDGTextureDesc::Create2D(
				TextureSize,
				RequiredOutputPixelFormat,
				FClearValueBinding::Black,
				TexCreate_ShaderResource | TexCreate_UAV);
			FRDGTextureRef DummyInputTexture = GraphBuilder.CreateTexture(DummyInputDesc, TEXT("CSDemo.FillRenderTargetRed.DummyInput"));
			AddClearUAVPass(GraphBuilder, GraphBuilder.CreateUAV(FRDGTextureUAVDesc(DummyInputTexture)), FLinearColor::Black);

			FRDGTextureRef OutputTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(OutputRenderTargetResource->GetTextureRHI(), TEXT("CSDemo.FillRenderTargetRed.Output")));

			FCSDemoRedFillCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FCSDemoRedFillCS::FParameters>();
			PassParameters->InputTexture = DummyInputTexture;
			PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));
			PassParameters->InputTextureSize = TextureSize;
			PassParameters->TextureSize = TextureSize;

			TShaderMapRef<FCSDemoRedFillCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("FillRenderTargetRedCS"),
				ComputeShader,
				PassParameters,
				FCSDemoRedFillCS::GetGroupCount(TextureSize));

			GraphBuilder.Execute();
		});
}

void UCSDemoComputeShaderLibrary::RunRedTestWithInput(UTextureRenderTarget2D* InputRenderTarget, UTextureRenderTarget2D* OutputRenderTarget)
{
	if (InputRenderTarget == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("RunRedTestWithInput called with a null input render target."));
		return;
	}

	if (!PrepareOutputRenderTarget(OutputRenderTarget))
	{
		return;
	}

	if (InputRenderTarget == OutputRenderTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("RunRedTestWithInput requires different input and output render targets."));
		return;
	}

	FTextureRenderTargetResource* InputRenderTargetResource = GetRenderTargetResourceChecked(InputRenderTarget, TEXT("RunRedTestWithInput"));
	FTextureRenderTargetResource* OutputRenderTargetResource = GetRenderTargetResourceChecked(OutputRenderTarget, TEXT("RunRedTestWithInput"));
	if (InputRenderTargetResource == nullptr || OutputRenderTargetResource == nullptr)
	{
		return;
	}

	const FIntPoint InputTextureSize(InputRenderTarget->SizeX, InputRenderTarget->SizeY);
	const FIntPoint OutputTextureSize(OutputRenderTarget->SizeX, OutputRenderTarget->SizeY);
	if (InputTextureSize.X <= 0 || InputTextureSize.Y <= 0 || OutputTextureSize.X <= 0 || OutputTextureSize.Y <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RunRedTestWithInput received an invalid input or output size."));
		return;
	}

	ENQUEUE_RENDER_COMMAND(CSDemoRunRedTestWithInput)(
		[InputRenderTargetResource, OutputRenderTargetResource, InputTextureSize, OutputTextureSize](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList, RDG_EVENT_NAME("CSDemo.RunRedTestWithInput"));
			FTextureRenderTarget2DResource* OutputRenderTarget2DResource = OutputRenderTargetResource->GetTextureRenderTarget2DResource();
			if (OutputRenderTarget2DResource == nullptr || !OutputRenderTarget2DResource->GetUnorderedAccessViewRHI().IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("RunRedTestWithInput aborted because the output render target does not expose a valid UAV."));
				return;
			}

			FRDGTextureRef InputTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(InputRenderTargetResource->GetTextureRHI(), TEXT("CSDemo.RunRedTestWithInput.Input")));
			FRDGTextureRef OutputTexture = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(OutputRenderTargetResource->GetTextureRHI(), TEXT("CSDemo.RunRedTestWithInput.Output")));

			FCSDemoRedFillCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FCSDemoRedFillCS::FParameters>();
			PassParameters->InputTexture = InputTexture;
			PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));
			PassParameters->InputTextureSize = InputTextureSize;
			PassParameters->TextureSize = OutputTextureSize;

			TShaderMapRef<FCSDemoRedFillCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("RunRedTestWithInputCS"),
				ComputeShader,
				PassParameters,
				FCSDemoRedFillCS::GetGroupCount(OutputTextureSize));

			GraphBuilder.Execute();
		});
}
