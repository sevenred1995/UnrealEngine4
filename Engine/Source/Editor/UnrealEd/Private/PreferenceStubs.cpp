// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "UnrealEd.h"

// @todo find a better place for all of this, preferably in the appropriate modules
// though this would require the classes to be relocated as well

//
// UCascadeOptions
// 
UCascadeOptions::UCascadeOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// UPhATSimOptions ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
UPhATSimOptions::UPhATSimOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PhysicsBlend = 1.0f;
	MaxFPS = -1;
}

UMaterialEditorOptions::UMaterialEditorOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UCurveEdOptions::UCurveEdOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UPersonaOptions::UPersonaOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DefaultLocalAxesSelection(2)
{
	ViewModeIndex = VMI_Lit;

	SectionTimingNodeColour = FLinearColor(0.0f, 1.0f, 0.0f);
	NotifyTimingNodeColour = FLinearColor(1.0f, 0.0f, 0.0f);
	BranchingPointTimingNodeColour = FLinearColor(0.5f, 1.0f, 1.0f);
}

void UPersonaOptions::SetViewportBackgroundColor( const FLinearColor& InViewportBackgroundColor)
{
	ViewportBackgroundColor = InViewportBackgroundColor;
	SaveConfig();
}

void UPersonaOptions::SetShowGrid( bool bInShowGrid )
{
	bShowGrid = bInShowGrid;
	SaveConfig();
}

void UPersonaOptions::SetHighlightOrigin( bool bInHighlightOrigin )
{
	bHighlightOrigin = bInHighlightOrigin;
	SaveConfig();
}

void UPersonaOptions::SetGridSize( int32 InGridSize )
{
	GridSize = InGridSize;
	SaveConfig();
}

void UPersonaOptions::SetViewModeIndex( EViewModeIndex InViewModeIndex )
{
	ViewModeIndex = InViewModeIndex;
	SaveConfig();
}

void UPersonaOptions::SetShowFloor( bool bInShowFloor )
{
	bShowFloor = bInShowFloor;
	SaveConfig();
}
void UPersonaOptions::SetShowSky( bool bInShowSky )
{
	bShowSky = bInShowSky;
	SaveConfig();
}

void UPersonaOptions::SetMuteAudio( bool bInMuteAudio )
{
	bMuteAudio = bInMuteAudio;
	SaveConfig();
}

void UPersonaOptions::SetViewFOV( float InViewFOV )
{
	ViewFOV = InViewFOV;
	SaveConfig();
}

void UPersonaOptions::SetDefaultLocalAxesSelection( uint32 InDefaultLocalAxesSelection )
{
	DefaultLocalAxesSelection = InDefaultLocalAxesSelection;
	SaveConfig();
}


void UPersonaOptions::SetShowMeshStats( int32 InShowMeshStats )
{
	ShowMeshStats = InShowMeshStats;
	SaveConfig();
}


void UPersonaOptions::SetSectionTimingNodeColor(const FLinearColor& InColor)
{
	SectionTimingNodeColour = InColor;
	SaveConfig();
}

void UPersonaOptions::SetNotifyTimingNodeColor(const FLinearColor& InColor)
{
	NotifyTimingNodeColour = InColor;
	SaveConfig();
}

void UPersonaOptions::SetBranchingPointTimingNodeColor(const FLinearColor& InColor)
{
	BranchingPointTimingNodeColour = InColor;
	SaveConfig();
}
