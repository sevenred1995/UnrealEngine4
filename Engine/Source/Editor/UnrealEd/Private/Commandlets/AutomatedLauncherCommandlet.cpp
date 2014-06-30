// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UnrealEd.h"
#include "Interfaces/ILauncherAutomatedServiceProvider.h"
#include "Interfaces/ILauncherAutomatedServiceModule.h"

UAutomatedLauncherCommandlet::UAutomatedLauncherCommandlet(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}


int32 UAutomatedLauncherCommandlet::Main( const FString& Params )
{
	// Setup the automated launcher service
	ILauncherAutomatedServiceModule& LauncherAutomatedServiceModule = FModuleManager::LoadModuleChecked<ILauncherAutomatedServiceModule>(TEXT( "LauncherAutomatedService"));
	
	ILauncherAutomatedServiceProviderPtr LauncherAutomatedService = LauncherAutomatedServiceModule.CreateAutomatedServiceProvider();
	LauncherAutomatedService->Setup( *Params );

	// Enter main loop
	double DeltaTime = 0.0;
	double LastTime = FPlatformTime::Seconds();
	
	GIsRequestingExit = !LauncherAutomatedService->IsRunning();
	while( !GIsRequestingExit )
	{
		// Tick any systems which require it
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);

		LauncherAutomatedService->Tick( DeltaTime );

		DeltaTime = FPlatformTime::Seconds() - LastTime;
		LastTime = FPlatformTime::Seconds();

		GIsRequestingExit = !LauncherAutomatedService->IsRunning();
	}



	// Shutdown our service
	LauncherAutomatedService->Shutdown();
	int32 ReturnCode = LauncherAutomatedService->GetExitCode();

	// Cleanup our module usage
	LauncherAutomatedServiceModule.ShutdownModule();

	return ReturnCode;
}
