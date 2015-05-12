// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	DistanceFieldLightingPost.h
=============================================================================*/

#pragma once

extern void AllocateOrReuseAORenderTarget(TRefCountPtr<IPooledRenderTarget>& Target, const TCHAR* Name, bool bWithAlpha);

extern void UpdateHistory(
	FRHICommandList& RHICmdList,
	const FViewInfo& View, 
	const TCHAR* BentNormalHistoryRTName,
	const TCHAR* IrradianceHistoryRTName,
	FSceneRenderTargetItem& VelocityTexture,
	/** Contains last frame's history, if non-NULL.  This will be updated with the new frame's history. */
	TRefCountPtr<IPooledRenderTarget>* BentNormalHistoryState,
	TRefCountPtr<IPooledRenderTarget>* IrradianceHistoryState,
	/** Source */
	TRefCountPtr<IPooledRenderTarget>& BentNormalSource, 
	TRefCountPtr<IPooledRenderTarget>& IrradianceSource, 
	/** Output of Temporal Reprojection for the next step in the pipeline. */
	TRefCountPtr<IPooledRenderTarget>& BentNormalHistoryOutput,
	TRefCountPtr<IPooledRenderTarget>& IrradianceHistoryOutput);

extern void PostProcessBentNormalAOSurfaceCache(
	FRHICommandList& RHICmdList, 
	const FDistanceFieldAOParameters& Parameters, 
	const FViewInfo& View, 
	FSceneRenderTargetItem& VelocityTexture,
	FSceneRenderTargetItem& BentNormalInterpolation, 
	IPooledRenderTarget* IrradianceInterpolation,
	FSceneRenderTargetItem& DistanceFieldNormal,
	TRefCountPtr<IPooledRenderTarget>& BentNormalOutput,
	TRefCountPtr<IPooledRenderTarget>& IrradianceOutput);

extern void UpsampleBentNormalAO(
	FRHICommandList& RHICmdList, 
	const TArray<FViewInfo>& Views, 
	TRefCountPtr<IPooledRenderTarget>& DistanceFieldAOBentNormal, 
	TRefCountPtr<IPooledRenderTarget>& DistanceFieldIrradiance,
	bool bVisualizeAmbientOcclusion,
	bool bVisualizeGlobalIllumination);