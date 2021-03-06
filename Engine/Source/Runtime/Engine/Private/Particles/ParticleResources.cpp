// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	ParticleResources.cpp: Implementation of global particle resources.
=============================================================================*/

#include "EnginePrivate.h"
#include "ParticleResources.h"

/** The size of the scratch vertex buffer. */
const int32 GParticleScratchVertexBufferSize = 64 * (1 << 10); // 64KB

/**
 * Creates a vertex buffer holding texture coordinates for the four corners of a sprite.
 */
void FParticleTexCoordVertexBuffer::InitRHI()
{
	const uint32 Size = sizeof(FVector2D) * 4 * MAX_PARTICLES_PER_INSTANCE;
	FRHIResourceCreateInfo CreateInfo;
	VertexBufferRHI = RHICreateVertexBuffer( Size, BUF_Static, CreateInfo);
	FVector2D* Vertices = (FVector2D*)RHILockVertexBuffer( VertexBufferRHI, 0, Size, RLM_WriteOnly );
	for (uint32 SpriteIndex = 0; SpriteIndex < MAX_PARTICLES_PER_INSTANCE; ++SpriteIndex)
	{
		Vertices[SpriteIndex*4 + 0] = FVector2D(0.0f, 0.0f);
		Vertices[SpriteIndex*4 + 1] = FVector2D(0.0f, 1.0f);
		Vertices[SpriteIndex*4 + 2] = FVector2D(1.0f, 1.0f);
		Vertices[SpriteIndex*4 + 3] = FVector2D(1.0f, 0.0f);
	}
	RHIUnlockVertexBuffer( VertexBufferRHI );
}

/** Global particle texture coordinate vertex buffer. */
TGlobalResource<FParticleTexCoordVertexBuffer> GParticleTexCoordVertexBuffer;

/**
 * Creates an index buffer for drawing an individual sprite.
 */
void FParticleIndexBuffer::InitRHI()
{
	// Instanced path needs only MAX_PARTICLES_PER_INSTANCE,
	// but using the maximum needed for the non-instanced path
	// in prep for future flipping of both instanced and non-instanced at runtime.
	const uint32 MaxParticles = 65536 / 4;
	const uint32 Size = sizeof(uint16) * 6 * MaxParticles;
	const uint32 Stride = sizeof(uint16);
	FRHIResourceCreateInfo CreateInfo;
	IndexBufferRHI = RHICreateIndexBuffer( Stride, Size, BUF_Static, CreateInfo);
	uint16* Indices = (uint16*)RHILockIndexBuffer( IndexBufferRHI, 0, Size, RLM_WriteOnly );
	for (uint32 SpriteIndex = 0; SpriteIndex < MaxParticles; ++SpriteIndex)
	{
		Indices[SpriteIndex*6 + 0] = SpriteIndex*4 + 0;
		Indices[SpriteIndex*6 + 1] = SpriteIndex*4 + 2;
		Indices[SpriteIndex*6 + 2] = SpriteIndex*4 + 3;
		Indices[SpriteIndex*6 + 3] = SpriteIndex*4 + 0;
		Indices[SpriteIndex*6 + 4] = SpriteIndex*4 + 1;
		Indices[SpriteIndex*6 + 5] = SpriteIndex*4 + 2;
	}
	RHIUnlockIndexBuffer( IndexBufferRHI );
}

/** Global particle index buffer. */
TGlobalResource<FParticleIndexBuffer> GParticleIndexBuffer;

/**
 * Creates a scratch vertex buffer available for dynamic draw calls.
 */
void FParticleScratchVertexBuffer::InitRHI()
{
	// Create a scratch vertex buffer for injecting particles and rendering tiles.
	uint32 Flags = BUF_Volatile;
	if (GetFeatureLevel() >= ERHIFeatureLevel::ES3_1)
	{
		Flags |= BUF_ShaderResource;
	}
	FRHIResourceCreateInfo CreateInfo;
	VertexBufferRHI = RHICreateVertexBuffer(GParticleScratchVertexBufferSize, Flags, CreateInfo);
	if (GetFeatureLevel() >= ERHIFeatureLevel::ES3_1)
	{
		VertexBufferSRV_G32R32F = RHICreateShaderResourceView( VertexBufferRHI, /*Stride=*/ sizeof(FVector2D), PF_G32R32F );
	}
}

FParticleShaderParamRef FParticleScratchVertexBuffer::GetShaderParam()
{
	return VertexBufferSRV_G32R32F;
}

FParticleBufferParamRef FParticleScratchVertexBuffer::GetBufferParam()
{
	return VertexBufferRHI;
}

/** Release RHI resources. */
void FParticleScratchVertexBuffer::ReleaseRHI()
{
	VertexBufferSRV_G32R32F.SafeRelease();
	FVertexBuffer::ReleaseRHI();
}

/** The global scratch vertex buffer. */
TGlobalResource<FParticleScratchVertexBuffer> GParticleScratchVertexBuffer;
