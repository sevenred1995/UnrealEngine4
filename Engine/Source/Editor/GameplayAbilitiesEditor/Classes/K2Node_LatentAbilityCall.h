// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "K2Node_LatentGameplayTaskCall.h"
#include "K2Node_LatentAbilityCall.generated.h"

UCLASS()
class UK2Node_LatentAbilityCall : public UK2Node_LatentGameplayTaskCall
{
	GENERATED_BODY()

public:
	UK2Node_LatentAbilityCall(const FObjectInitializer& ObjectInitializer);

	// UEdGraphNode interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsCompatibleWithGraph(UEdGraph const* TargetGraph) const override;
	// End of UEdGraphNode interface
	
protected:
	virtual bool IsHandling(TSubclassOf<UGameplayTask> TaskClass) const override;
};
