// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

/*-----------------------------------------------------------------------------
FNiagaraEmitterPropertiesDetails
-----------------------------------------------------------------------------*/

class FNiagaraEmitterPropertiesDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<class IDetailCustomization> MakeInstance(TWeakObjectPtr<UNiagaraEmitterProperties> EmitterProperties);

	/** Constructor */
	FNiagaraEmitterPropertiesDetails(TWeakObjectPtr<UNiagaraEmitterProperties> EmitterProperties);

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	void OnGenerateScalarConstantEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder);
	void OnGenerateVectorConstantEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder);
	void OnGenerateMatrixConstantEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder);
	void OnGenerateDataObjectConstantEntry(TSharedRef<IPropertyHandle> ElementProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder);

	void BuildScriptProperties(IDetailLayoutBuilder& DetailLayout, TSharedRef<IPropertyHandle> ScriptPropsHandle, FName Name, FText DisplayName);
private:
	/** Object that stores all of the possible parameters we can edit */
	TWeakObjectPtr<UNiagaraEmitterProperties> EmitterProps;
};

