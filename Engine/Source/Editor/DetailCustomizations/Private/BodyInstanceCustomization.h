// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

struct FCollisionChannelInfo
{
	FString				DisplayName;
	ECollisionChannel	CollisionChannel;
	bool				TraceType;
};

/**
 * Customizes a DataTable asset to use a dropdown
 */
class FBodyInstanceCustomization : public IPropertyTypeCustomization
{
public:
	FBodyInstanceCustomization();

	static TSharedRef<IPropertyTypeCustomization> MakeInstance() 
	{
		return MakeShareable( new FBodyInstanceCustomization );
	}

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader( TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override {};
	virtual void CustomizeChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;

private:
	// Profile combo related
	TSharedRef<SWidget> MakeCollisionProfileComboWidget( TSharedPtr<FString> InItem );
	void OnCollisionProfileChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo, IDetailGroup* CollisionGroup );
	FText GetCollisionProfileComboBoxContent() const;
	FText GetCollisionProfileComboBoxToolTip() const;
	void OnCollisionProfileComboOpening();

	// Movement channel related
	TSharedRef<SWidget> MakeObjectTypeComboWidget( TSharedPtr<FString> InItem );
	void OnObjectTypeChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo  );
	FText GetObjectTypeComboBoxContent() const;
	int32 InitializeObjectTypeComboList();

	// set to default for profile setting
	FReply SetToDefaultProfile();
	EVisibility ShouldShowResetToDefaultProfile() const;

	FReply SetToDefaultResponse(int32 ValidIndex);
	EVisibility ShouldShowResetToDefaultResponse(int32 ValidIndex) const;

	// collision channel check boxes
	void OnCollisionChannelChanged(ECheckBoxState InNewValue, int32 ValidIndex, ECollisionResponse InCollisionResponse);
	ECheckBoxState IsCollisionChannelChecked( int32 ValidIndex, ECollisionResponse InCollisionResponse) const;
	// all collision channel check boxes
	void OnAllCollisionChannelChanged(ECheckBoxState InNewValue, ECollisionResponse InCollisionResponse);
	ECheckBoxState IsAllCollisionChannelChecked( ECollisionResponse InCollisionResponse) const;

	// should show custom prop
	bool ShouldEnableCustomCollisionSetup() const;
	EVisibility ShouldShowCustomCollisionSetup() const;

	// utility functions between property and struct
	void CreateCustomCollisionSetup( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailGroup& CollisionGroup );
	void SetCollisionResponseContainer(const FCollisionResponseContainer& ResponseContainer);
	void SetResponse(int32 ValidIndex, ECollisionResponse InCollisionResponse);
	void UpdateCollisionProfile();
	TSharedPtr<FString> GetProfileString(FName ProfileName) const;

	void UpdateValidCollisionChannels();

private:
	// property handles
	TSharedPtr<IPropertyHandle> CollisionProfileNameHandle;
	TSharedPtr<IPropertyHandle> CollisionEnabledHandle;
	TSharedPtr<IPropertyHandle> ObjectTypeHandle;
	TSharedPtr<IPropertyHandle> CollisionResponsesHandle;

	// widget related variables
	TSharedPtr<class SComboBox< TSharedPtr<FString> > > CollsionProfileComboBox;
	TArray< TSharedPtr< FString > >						CollisionProfileComboList;

	// movement channel related options
	TSharedPtr<class SComboBox< TSharedPtr<FString> > > ObjectTypeComboBox;
	TArray< TSharedPtr< FString > >						ObjectTypeComboList;
	// matching ObjectType value to ComboList, technically you can search DisplayName all the time, but this seems just easier
	TArray< ECollisionChannel >							ObjectTypeValues; 

	// default collision profile object
	UCollisionProfile * CollisionProfile;

	TArray<FBodyInstance*>		BodyInstances;

	TArray<FCollisionChannelInfo>	ValidCollisionChannels;

	void RefreshCollisionProfiles();
};
