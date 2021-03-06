// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "EnvironmentQueryEditorPrivatePCH.h"
#include "EnvironmentQueryEditorModule.h"

//////////////////////////////////////////////////////////////////////////
// EnvironmentQueryGraph

namespace EQSGraphVersion
{
	const int32 Initial = 0;
	const int32 NestedNodes = 1;
	const int32 CopyPasteOutersBug = 2;
	const int32 BlueprintClasses = 3;

	const int32 Latest = BlueprintClasses;
}

UEnvironmentQueryGraph::UEnvironmentQueryGraph(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Schema = UEdGraphSchema_EnvironmentQuery::StaticClass();
}

struct FCompareNodeXLocation
{
	FORCEINLINE bool operator()(const UEdGraphPin& A, const UEdGraphPin& B) const
	{
		return A.GetOwningNode()->NodePosX < B.GetOwningNode()->NodePosX;
	}
};

void UEnvironmentQueryGraph::UpdateAsset(int32 UpdateFlags)
{
	if (IsLocked())
	{
		return;
	}

	// let's find root node
	UEnvironmentQueryGraphNode_Root* RootNode = NULL;
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		RootNode = Cast<UEnvironmentQueryGraphNode_Root>(Nodes[Idx]);
		if (RootNode != NULL)
		{
			break;
		}
	}

	UEnvQuery* Query = Cast<UEnvQuery>(GetOuter());
	Query->Options.Reset();
	if (RootNode && RootNode->Pins.Num() > 0 && RootNode->Pins[0]->LinkedTo.Num() > 0)
	{
		UEdGraphPin* MyPin = RootNode->Pins[0];

		// sort connections so that they're organized the same as user can see in the editor
		MyPin->LinkedTo.Sort(FCompareNodeXLocation());

		for (int32 Idx = 0; Idx < MyPin->LinkedTo.Num(); Idx++)
		{
			UEnvironmentQueryGraphNode_Option* OptionNode = Cast<UEnvironmentQueryGraphNode_Option>(MyPin->LinkedTo[Idx]->GetOwningNode());
			if (OptionNode)
			{
				OptionNode->UpdateNodeData();

				UEnvQueryOption* OptionInstance = Cast<UEnvQueryOption>(OptionNode->NodeInstance);
				if (OptionInstance && OptionInstance->Generator)
				{
					OptionInstance->Tests.Reset();

					for (int32 TestIdx = 0; TestIdx < OptionNode->SubNodes.Num(); TestIdx++)
					{
						UAIGraphNode* SubNode = OptionNode->SubNodes[TestIdx];
						if (SubNode == nullptr)
						{
							continue;
						}

						SubNode->ParentNode = OptionNode;

						UEnvironmentQueryGraphNode_Test* TestNode = Cast<UEnvironmentQueryGraphNode_Test>(SubNode);
						if (TestNode && TestNode->bTestEnabled)
						{
							UEnvQueryTest* TestInstance = Cast<UEnvQueryTest>(TestNode->NodeInstance);
							if (TestInstance)
							{
								OptionInstance->Tests.Add(TestInstance);
							}
						}
					}

					Query->Options.Add(OptionInstance);
				}
			}
		}
	}

	RemoveOrphanedNodes();
#if USE_EQS_DEBUGGER
	UEnvQueryManager::NotifyAssetUpdate(Query);
#endif
}

void UEnvironmentQueryGraph::Initialize()
{
	Super::Initialize();
	
	LockUpdates();
	SpawnMissingNodes();
	CalculateAllWeights();
	UnlockUpdates();
}

void UEnvironmentQueryGraph::OnLoaded()
{
	Super::OnLoaded();
	UpdateDeprecatedGeneratorClasses();
}

void UEnvironmentQueryGraph::CalculateAllWeights()
{
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UEnvironmentQueryGraphNode_Option* OptionNode = Cast<UEnvironmentQueryGraphNode_Option>(Nodes[Idx]);
		if (OptionNode)
		{
			OptionNode->CalculateWeights();
		}
	}
}

void UEnvironmentQueryGraph::MarkVersion()
{
	GraphVersion = EQSGraphVersion::Latest;
}

void UEnvironmentQueryGraph::UpdateVersion()
{
	if (GraphVersion == EQSGraphVersion::Latest)
	{
		return;
	}

	// convert to nested nodes
	if (GraphVersion < EQSGraphVersion::NestedNodes)
	{
		UpdateVersion_NestedNodes();
	}

	if (GraphVersion < EQSGraphVersion::CopyPasteOutersBug)
	{
		UpdateVersion_FixupOuters();
	}

	if (GraphVersion < EQSGraphVersion::BlueprintClasses)
	{
		UpdateVersion_CollectClassData();
	}

	GraphVersion = EQSGraphVersion::Latest;
	Modify();
}

void UEnvironmentQueryGraph::UpdateVersion_NestedNodes()
{
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UEnvironmentQueryGraphNode_Option* OptionNode = Cast<UEnvironmentQueryGraphNode_Option>(Nodes[Idx]);
		if (OptionNode)
		{
			UEnvironmentQueryGraphNode* Node = OptionNode;
			while (Node)
			{
				UEnvironmentQueryGraphNode* NextNode = NULL;
				for (int32 iPin = 0; iPin < Node->Pins.Num(); iPin++)
				{
					UEdGraphPin* TestPin = Node->Pins[iPin];
					if (TestPin && TestPin->Direction == EGPD_Output)
					{
						for (int32 iLink = 0; iLink < TestPin->LinkedTo.Num(); iLink++)
						{
							UEdGraphPin* LinkedTo = TestPin->LinkedTo[iLink];
							UEnvironmentQueryGraphNode_Test* LinkedTest = LinkedTo ? Cast<UEnvironmentQueryGraphNode_Test>(LinkedTo->GetOwningNode()) : NULL;
							if (LinkedTest)
							{
								LinkedTest->ParentNode = OptionNode;
								OptionNode->SubNodes.Add(LinkedTest);

								NextNode = LinkedTest;
								break;
							}
						}

						break;
					}
				}

				Node = NextNode;
			}
		}
	}

	for (int32 Idx = Nodes.Num() - 1; Idx >= 0; Idx--)
	{
		UEnvironmentQueryGraphNode_Test* TestNode = Cast<UEnvironmentQueryGraphNode_Test>(Nodes[Idx]);
		if (TestNode)
		{
			TestNode->Pins.Empty();
			Nodes.RemoveAt(Idx);
			continue;
		}

		UEnvironmentQueryGraphNode_Option* OptionNode = Cast<UEnvironmentQueryGraphNode_Option>(Nodes[Idx]);
		if (OptionNode && OptionNode->Pins.IsValidIndex(1))
		{
			OptionNode->Pins.RemoveAt(1);
		}
	}
}

void UEnvironmentQueryGraph::UpdateVersion_FixupOuters()
{
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UEnvironmentQueryGraphNode* MyNode = Cast<UEnvironmentQueryGraphNode>(Nodes[Idx]);
		if (MyNode)
		{
			MyNode->PostEditImport();
		}
	}
}

void UEnvironmentQueryGraph::UpdateVersion_CollectClassData()
{
	UpdateClassData();
}

void UEnvironmentQueryGraph::CollectAllNodeInstances(TSet<UObject*>& NodeInstances)
{
	Super::CollectAllNodeInstances(NodeInstances);

	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UEnvironmentQueryGraphNode* MyNode = Cast<UEnvironmentQueryGraphNode>(Nodes[Idx]);
		UEnvQueryOption* OptionInstance = MyNode ? Cast<UEnvQueryOption>(MyNode->NodeInstance) : nullptr;
		if (OptionInstance && OptionInstance->Generator)
		{
			NodeInstances.Add(OptionInstance->Generator);
		}
	}
}

void UEnvironmentQueryGraph::UpdateDeprecatedGeneratorClasses()
{
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UEnvironmentQueryGraphNode* MyNode = Cast<UEnvironmentQueryGraphNode>(Nodes[Idx]);
		UEnvQueryOption* OptionInstance = MyNode ? Cast<UEnvQueryOption>(MyNode->NodeInstance) : nullptr;
		if (OptionInstance && OptionInstance->Generator)
		{
			MyNode->ErrorMessage = FGraphNodeClassHelper::GetDeprecationMessage(OptionInstance->Generator->GetClass());
		}
	}
}

void UEnvironmentQueryGraph::SpawnMissingNodes()
{
	UEnvQuery* QueryOwner = Cast<UEnvQuery>(GetOuter());
	if (QueryOwner == nullptr)
	{
		return;
	}

	TSet<UEnvQueryTest*> ExistingTests;
	TSet<UEnvQueryOption*> ExistingNodes;
	TArray<UEnvQueryOption*> OptionsCopy = QueryOwner->Options;

	UAIGraphNode* MyRootNode = nullptr;
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		UEnvironmentQueryGraphNode* MyNode = Cast<UEnvironmentQueryGraphNode>(Nodes[Idx]);
		UEnvQueryOption* OptionInstance = MyNode ? Cast<UEnvQueryOption>(MyNode->NodeInstance) : nullptr;
		if (OptionInstance && OptionInstance->Generator)
		{
			ExistingNodes.Add(OptionInstance);

			ExistingTests.Empty(ExistingTests.Num());
			for (int32 SubIdx = 0; SubIdx < MyNode->SubNodes.Num(); SubIdx++)
			{
				UEnvironmentQueryGraphNode* MySubNode = Cast<UEnvironmentQueryGraphNode>(MyNode->SubNodes[SubIdx]);
				UEnvQueryTest* TestInstance = MySubNode ? Cast<UEnvQueryTest>(MySubNode->NodeInstance) : nullptr;
				if (TestInstance)
				{
					ExistingTests.Add(TestInstance);
				}
				else
				{
					MyNode->RemoveSubNode(MySubNode);
					SubIdx--;
				}
			}

			SpawnMissingSubNodes(OptionInstance, ExistingTests, MyNode);
		}

		UEnvironmentQueryGraphNode_Root* RootNode = Cast<UEnvironmentQueryGraphNode_Root>(Nodes[Idx]);
		if (RootNode)
		{
			MyRootNode = RootNode;
		}
	}

	UEdGraphPin* RootOutPin = MyRootNode ? FindGraphNodePin(MyRootNode, EGPD_Output) : nullptr;
	ExistingTests.Empty(0);

	for (int32 Idx = 0; Idx < OptionsCopy.Num(); Idx++)
	{
		UEnvQueryOption* OptionInstance = OptionsCopy[Idx];
		if (ExistingNodes.Contains(OptionInstance) || OptionInstance == nullptr || OptionInstance->Generator == nullptr)
		{
			continue;
		}

		FGraphNodeCreator<UEnvironmentQueryGraphNode_Option> NodeBuilder(*this);
		UEnvironmentQueryGraphNode_Option* MyNode = NodeBuilder.CreateNode();
		UAIGraphNode::UpdateNodeClassDataFrom(OptionInstance->Generator->GetClass(), MyNode->ClassData);
		MyNode->ErrorMessage = MyNode->ClassData.GetDeprecatedMessage();
		NodeBuilder.Finalize();

		if (MyRootNode)
		{
			MyNode->NodePosX = MyRootNode->NodePosX + (Idx * 300);
			MyNode->NodePosY = MyRootNode->NodePosY + 100;
		}

		MyNode->NodeInstance = OptionInstance;
		SpawnMissingSubNodes(OptionInstance, ExistingTests, MyNode);

		UEdGraphPin* SpawnedInPin = FindGraphNodePin(MyNode, EGPD_Input);
		if (RootOutPin && SpawnedInPin)
		{
			RootOutPin->MakeLinkTo(SpawnedInPin);
		}
	}
}

void UEnvironmentQueryGraph::SpawnMissingSubNodes(UEnvQueryOption* Option, TSet<UEnvQueryTest*> ExistingTests, UEnvironmentQueryGraphNode* OptionNode)
{
	TArray<UEnvQueryTest*> TestsCopy = Option->Tests;
	for (int32 SubIdx = 0; SubIdx < TestsCopy.Num(); SubIdx++)
	{
		if (ExistingTests.Contains(TestsCopy[SubIdx]) || (TestsCopy[SubIdx] == nullptr))
		{
			continue;
		}

		UEnvironmentQueryGraphNode_Test* TestNode = NewObject<UEnvironmentQueryGraphNode_Test>(this);
		TestNode->NodeInstance = TestsCopy[SubIdx];
		TestNode->UpdateNodeClassData();

		OptionNode->AddSubNode(TestNode, this);
		TestNode->NodeInstance = TestsCopy[SubIdx];
	}
}
