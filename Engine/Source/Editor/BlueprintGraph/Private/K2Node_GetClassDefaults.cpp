// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "BlueprintGraphPrivatePCH.h"
#include "EditorCategoryUtils.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "KismetCompiler.h"
#include "K2Node_GetClassDefaults.h"

#define LOCTEXT_NAMESPACE "UK2Node_GetClassDefaults"

FString UK2Node_GetClassDefaults::ClassPinName(TEXT("Class"));

namespace
{
	// Optional pin manager subclass.
	struct FClassDefaultsOptionalPinManager : public FOptionalPinManager
	{
		FClassDefaultsOptionalPinManager(UClass* InClass)
			:FOptionalPinManager()
		{
			SrcClass = InClass;
		}

		virtual void GetRecordDefaults(UProperty* TestProperty, FOptionalPinFromProperty& Record) const override
		{
			FOptionalPinManager::GetRecordDefaults(TestProperty, Record);

			// Show pin unless the property is owned by a parent class.
			Record.bShowPin = TestProperty->GetOwnerClass() == SrcClass;
		}

		virtual bool CanTreatPropertyAsOptional(UProperty* TestProperty) const override
		{
			// Don't expose object reference properties or anything not marked BlueprintReadOnly/BlueprintReadWrite.
			// @TODO - Could potentially expose object reference values if/when we have support for 'const' input pins.
			return TestProperty != nullptr
				&& !TestProperty->IsA<UClassProperty>()
				&& !TestProperty->IsA<UObjectProperty>()
				&& !TestProperty->IsA<UInterfaceProperty>()
				&& TestProperty->HasAllPropertyFlags(CPF_BlueprintVisible);
		}

		virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex, UProperty* Property = nullptr) const override
		{
			check(Pin);

			// Move into the advanced view if the property metadata is set.
			Pin->bAdvancedView = Property && Property->HasAnyPropertyFlags(CPF_AdvancedDisplay);
		}

	private:
		// Class type for which optional pins are being managed.
		UClass* SrcClass;
	};

	// Compilation handler subclass.
	class FKCHandler_GetClassDefaults : public FNodeHandlingFunctor
	{
	public:
		FKCHandler_GetClassDefaults(FKismetCompilerContext& InCompilerContext)
			: FNodeHandlingFunctor(InCompilerContext)
		{
		}

		virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
		{
			// Cast to the correct node type
			if(const UK2Node_GetClassDefaults* GetClassDefaultsNode = Cast<UK2Node_GetClassDefaults>(Node))
			{
				// Only if we have a valid class input pin
				if(UEdGraphPin* ClassPin = GetClassDefaultsNode->FindClassPin())
				{
					// Redirect to a linked pin if necessary
					UEdGraphPin* Net = FEdGraphUtilities::GetNetFromPin(ClassPin);
					check(Net != nullptr);

					// Register a literal if necessary (e.g. there are no linked pins)
					if(ValidateAndRegisterNetIfLiteral(Context, Net))
					{
						// First check for a literal term in case one was created above
						FBPTerminal** FoundTerm = Context.LiteralHackMap.Find(Net);
						if(FoundTerm == nullptr)
						{
							// Otherwise, check for a linked term
							FoundTerm = Context.NetMap.Find(Net);
						}

						// If we did not find an input term, make sure we create one here
						FBPTerminal* ClassContextTerm = FoundTerm ? *FoundTerm : nullptr;
						if(ClassContextTerm == nullptr)
						{
							ClassContextTerm = Context.CreateLocalTerminalFromPinAutoChooseScope(Net, Context.NetNameMap->MakeValidName(Net));
							check(ClassContextTerm != nullptr);

							Context.NetMap.Add(Net, ClassContextTerm);
						}

						// Flag this as a "class context" term
						ClassContextTerm->SetContextTypeClass();

						// Infer the class type from the context term
						if(const UClass* ClassType = Cast<UClass>(ClassContextTerm->bIsLiteral ? ClassContextTerm->ObjectLiteral : ClassContextTerm->Type.PinSubCategoryObject.Get()))
						{
							// Create a local term for each output pin (class property)
							for(int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
							{
								UEdGraphPin* Pin = Node->Pins[PinIndex];
								if(Pin != nullptr && Pin->Direction == EGPD_Output)
								{
									UProperty* BoundProperty = FindField<UProperty>(ClassType, *(Pin->PinName));
									if(BoundProperty != nullptr)
									{
										FBPTerminal* OutputTerm = Context.CreateLocalTerminalFromPinAutoChooseScope(Pin, Pin->PinName);
										check(OutputTerm != nullptr);

										// Set as a variable within the class context
										OutputTerm->AssociatedVarProperty = BoundProperty;
										OutputTerm->Context = ClassContextTerm;

										// Flag this as a "class default" variable term
										OutputTerm->bIsConst = true;
										OutputTerm->SetVarTypeDefault();

										// Add it to the lookup table
										Context.NetMap.Add(Pin, OutputTerm);
									}
									else
									{
										CompilerContext.MessageLog.Error(*LOCTEXT("UnmatchedOutputPinOnCompile", "Failed to find a class member to match @@").ToString(), Pin);
									}
								}
							}
						}
						else
						{
							CompilerContext.MessageLog.Error(*LOCTEXT("InvalidClassTypeOnCompile", "Missing or invalid input class type for @@").ToString(), Node);
						}
					}
				}
			}
		}
	};
}

UK2Node_GetClassDefaults::UK2Node_GetClassDefaults(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UK2Node_GetClassDefaults::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Rebuild the node if the "Show Pin" property was changed
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if((PropertyName == TEXT("bShowPin")))
	{
		GetSchema()->ReconstructNode(*this);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FText UK2Node_GetClassDefaults::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// @TODO - unique node icon needed as well?

	return LOCTEXT("NodeTitle", "Get Class Defaults");
}

void UK2Node_GetClassDefaults::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	// Create the class input type selector pin
	UEdGraphPin* ClassPin = CreatePin(EGPD_Input, K2Schema->PC_Class, TEXT(""), UObject::StaticClass(), false, false, ClassPinName);
	K2Schema->ConstructBasicPinTooltip(*ClassPin, LOCTEXT("ClassPinDescription", "The class from which to access one or more default values."), ClassPin->PinToolTip);
}

void UK2Node_GetClassDefaults::PostPlacedNewNode()
{
	if(UEdGraphPin* ClassPin = FindClassPin(Pins))
	{
		// Default to the owner BP's generated class for "normal" BPs if this is a new node
		const UBlueprint* OwnerBlueprint = GetBlueprint();
		if(OwnerBlueprint != nullptr && OwnerBlueprint->BlueprintType == BPTYPE_Normal)
		{
			ClassPin->DefaultObject = OwnerBlueprint->GeneratedClass;
		}

		if(UClass* InputClass = GetInputClass(ClassPin))
		{
			CreateOutputPins(InputClass);
		}
	}
}

void UK2Node_GetClassDefaults::PinConnectionListChanged(UEdGraphPin* ChangedPin)
{
	if(ChangedPin != nullptr && ChangedPin->PinName == ClassPinName && ChangedPin->Direction == EGPD_Input)
	{
		OnClassPinChanged();
	}
}

void UK2Node_GetClassDefaults::PinDefaultValueChanged(UEdGraphPin* ChangedPin) 
{
	if(ChangedPin != nullptr && ChangedPin->PinName == ClassPinName && ChangedPin->Direction == EGPD_Input)
	{
		OnClassPinChanged();
	}
}

void UK2Node_GetClassDefaults::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) 
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

	// Recreate output pins based on the previous input class
	UEdGraphPin* OldClassPin = FindClassPin(OldPins);
	if(UClass* InputClass = GetInputClass(OldClassPin))
	{
		CreateOutputPins(InputClass);
	}
}

FNodeHandlingFunctor* UK2Node_GetClassDefaults::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FKCHandler_GetClassDefaults(CompilerContext);
}

void UK2Node_GetClassDefaults::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_GetClassDefaults::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Class);
}

UEdGraphPin* UK2Node_GetClassDefaults::FindClassPin(const TArray<UEdGraphPin*>& FromPins) const
{
	UEdGraphPin* const* FoundPin = FromPins.FindByPredicate([](const UEdGraphPin* CurPin)
	{
		return CurPin && CurPin->Direction == EGPD_Input && CurPin->PinName == ClassPinName;
	});

	return FoundPin ? *FoundPin : nullptr;
}

UClass* UK2Node_GetClassDefaults::GetInputClass(const UEdGraphPin* FromPin) const
{
	UClass* InputClass = nullptr;

	const UEdGraphPin* ClassPin = FromPin;
	if(ClassPin == nullptr)
	{
		ClassPin = FindClassPin();
	}
	
	if(ClassPin != nullptr)
	{
		check(ClassPin->Direction == EGPD_Input);

		if(ClassPin->DefaultObject != nullptr && ClassPin->LinkedTo.Num() == 0)
		{
			InputClass = CastChecked<UClass>(ClassPin->DefaultObject);
		}
		else if(ClassPin->LinkedTo.Num() > 0)
		{
			if(UEdGraphPin* LinkedPin = ClassPin->LinkedTo[0])
			{
				InputClass = Cast<UClass>(LinkedPin->PinType.PinSubCategoryObject.Get());
			}
		}
	}

	return InputClass;
}

void UK2Node_GetClassDefaults::CreateOutputPins(UClass* InClass)
{
	// Create the set of output pins through the optional pin manager
	FClassDefaultsOptionalPinManager OptionalPinManager(InClass);
	OptionalPinManager.RebuildPropertyList(ShowPinForProperties, InClass);
	OptionalPinManager.CreateVisiblePins(ShowPinForProperties, InClass, EGPD_Output, this);

	// Check for any advanced properties (outputs)
	bool bHasAdvancedPins = false;
	for(int32 PinIndex = 0; PinIndex < Pins.Num() && !bHasAdvancedPins; ++PinIndex)
	{
		UEdGraphPin* Pin = Pins[PinIndex];
		check(Pin != nullptr);

		bHasAdvancedPins |= Pin->bAdvancedView;
	}

	// Toggle advanced display on/off based on whether or not we have any advanced outputs
	if(bHasAdvancedPins && AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
	{
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	}
	else if(!bHasAdvancedPins)
	{
		AdvancedPinDisplay = ENodeAdvancedPins::NoPins;
	}
}

void UK2Node_GetClassDefaults::OnClassPinChanged()
{
	TArray<UEdGraphPin*> OldPins = Pins;
	TArray<UEdGraphPin*> OldOutputPins;

	// Gather all current output pins
	for(int32 PinIndex = 0; PinIndex < OldPins.Num(); ++PinIndex)
	{
		UEdGraphPin* OldPin = OldPins[PinIndex];
		if(OldPin->Direction == EGPD_Output)
		{
			Pins.Remove(OldPin);
			OldOutputPins.Add(OldPin);
		}
	}

	// Clear the current output pin settings (so they don't carry over to the new set)
	ShowPinForProperties.Empty();

	// Create output pins for the new class type
	UClass* InputClass = GetInputClass();
	CreateOutputPins(InputClass);

	// Destroy the previous set of output pins
	DestroyPinList(OldOutputPins);

	// Notify the graph that the node has been changed
	if(UEdGraph* Graph = GetGraph())
	{
		Graph->NotifyGraphChanged();
	}
}

#undef LOCTEXT_NAMESPACE
