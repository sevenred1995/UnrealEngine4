// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interfaces/Interface_CollisionDataProvider.h"
#include "Components/SkinnedMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SkeletalMeshTypes.h"
#include "Animation/AnimationAsset.h"
#include "AnimCurveTypes.h"
#include "SkeletalMeshComponent.generated.h"

class UAnimInstance;
struct FEngineShowFlags;
struct FConvexVolume;

struct FAnimationEvaluationContext
{
	// The anim instance we are evaluating
	UAnimInstance* AnimInstance;

	// The SkeletalMesh we are evaluating for
	USkeletalMesh* SkeletalMesh;

	// Double buffer evaluation data
	TArray<FTransform> SpaceBases;
	TArray<FTransform> LocalAtoms;
	TArray<FActiveVertexAnim> VertexAnims;
	FVector RootBoneTranslation;

	// Double buffer curve data
	FBlendedCurve	Curve;

	// Are we performing interpolation this tick
	bool bDoInterpolation;

	// Are we evaluating this tick
	bool bDoEvaluation;

	// Are we storing data in cache bones this tick
	bool bDuplicateToCacheBones;

	// duplicate the cache curves
	bool bDuplicateToCacheCurve;

	FAnimationEvaluationContext()
	{
		Clear();
	}

	void Clear()
	{
		AnimInstance = NULL;
		SkeletalMesh = NULL;
	}

};

//#if WITH_APEX
namespace physx
{ 
	namespace apex 
	{
		class NxClothingAsset;
		class NxClothingActor;
		class NxClothingCollision;
	}
}
//#endif // WITH_APEX

class FPhysScene;

/** a class to manage an APEX clothing actor */
class FClothingActor
{
public:
	enum TeleportMode
	{
		/** Simulation continues smoothly. This is the most commonly used mode */
		Continuous,
		/**
		 * Transforms the current simulation state from the old global pose to the new global pose.
		 * This will transform positions and velocities and thus keep the simulation state, just translate it to a new pose.
		 */
		Teleport,

		/**
		 * Forces the cloth to the animated position in the next frame.
		 * This can be used to reset it from a bad state or by a teleport where the old state is not important anymore.
		 */
		TeleportAndReset,
	};

	void Clear(bool bReleaseResource = false);

	/** 
	 * to check whether this actor is valid or not 
	 * because clothing asset can be changed by editing 
	 */
	physx::apex::NxClothingAsset*	ParentClothingAsset;
	/** APEX clothing actor is created from APEX clothing asset for cloth simulation */
	physx::apex::NxClothingActor*		ApexClothingActor;
	FPhysScene * PhysScene;
	uint32 SceneType;
};

/** data for updating cloth section from the results of clothing simulation */
struct FClothSimulData
{
	TArray<FVector4> ClothSimulPositions;
	TArray<FVector4> ClothSimulNormals;
};

/**  for storing precomputed cloth morph target data */
struct FClothMorphTargetData
{
	FName MorphTargetName;
	// save a previous weight to compare whether weight was changed or not
	float PrevWeight;
	// an index of clothing assets which this morph target data is used in
	int32 ClothAssetIndex;
	// original positions which only this cloth section is including / extracted from a morph target
	TArray<FVector> OriginPos;
	// delta positions to morph this cloth section
	TArray<FVector> PosDelta;
};

#if WITH_CLOTH_COLLISION_DETECTION

class FClothCollisionPrimitive
{
public:
	enum ClothCollisionPrimType
	{
		SPHERE,
		CAPSULE,
		CONVEX,
		PLANE,		
	};

public:
	// for sphere and convex ( also used in debug draw of capsule )
	FVector Origin;
	// for sphere and capsule
	float	Radius;
	// for capsule ( needs 2 spheres to make an capsule, top end and bottom end )
	FVector SpherePos1;
	FVector SpherePos2;
	// for convex
	TArray<FPlane> ConvexPlanes;

	ClothCollisionPrimType	PrimType;
};

/** Used to define what type of objects cloth will react to. */
struct FApexClothCollisionInfo
{
	enum OverlappedComponentType
	{
		/** Interact with static objects only. */
		OCT_STATIC,
		/** Interact with clothing objects. */
		OCT_CLOTH,
		OCT_MAX
	};

	OverlappedComponentType OverlapCompType;
	/** To verify validation of collision info. */
	uint32 Revision;
	/** ClothingCollisions will be all released when clothing doesn't intersect with this component anymore. */
	TArray<physx::apex::NxClothingCollision*> ClothingCollisions;			
};
#endif // #if WITH_CLOTH_COLLISION_DETECTION

/** This enum defines how you'd like to update bones to physics world.
	If bone is simulating, you don't have to waste time on updating bone transform from kinematic.
	But also sometimes you don't like fixed bones to be updated by animation data. */
UENUM()
namespace EKinematicBonesUpdateToPhysics
{
	enum Type
	{
		/** Update any bones that are not simulating. */
		SkipSimulatingBones,
		/** Skip physics update from kinematic changes. */
		SkipAllBones
	};
}

UENUM()
namespace EAnimationMode
{
	enum Type
	{
		AnimationBlueprint UMETA(DisplayName="Use Animation Blueprint"), 
		AnimationSingleNode UMETA(DisplayName="Use Animation Asset")
	};
}

USTRUCT()
struct FSingleAnimationPlayData
{
	GENERATED_USTRUCT_BODY()

	// @todo in the future, we should make this one UObject
	// and have detail customization to display different things
	// The default sequence to play on this skeletal mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation)
	class UAnimationAsset* AnimToPlay;

	// @fixme : until we properly support it I'm commenting out editable property part
	// The default sequence to play on this skeletal mesh
	UPROPERTY()//EditAnywhere, BlueprintReadWrite, Category=Animation)
	class UVertexAnimation* VertexAnimToPlay;

	/** Default setting for looping for SequenceToPlay. This is not current state of looping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation, meta=(DisplayName="Looping"))
	uint32 bSavedLooping:1;

	/** Default setting for playing for SequenceToPlay. This is not current state of playing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation, meta=(DisplayName="Playing"))
	uint32 bSavedPlaying:1;

	/** Default setting for position of SequenceToPlay to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation, meta=(DisplayName="Initial Position"))
	float SavedPosition;

	/** Default setting for play rate of SequenceToPlay to play. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category=Animation, meta=(DisplayName="PlayRate"))
	float SavedPlayRate;

	FSingleAnimationPlayData()
	{
		SavedPlayRate = 1.0f;
		bSavedLooping = true;
		bSavedPlaying = true;
		SavedPosition = 0.f;
	}

	/** Called when initialized. */
	ENGINE_API void Initialize(class UAnimSingleNodeInstance* Instance);

	/** Populates this play data with the current state of the supplied instance. */
	ENGINE_API void PopulateFrom(UAnimSingleNodeInstance* Instance);

	void ValidatePosition();
};

/**
* Tick function that prepares for cloth tick
**/
USTRUCT()
struct FSkeletalMeshComponentPreClothTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

	/** World this tick function belongs to. */
	class USkeletalMeshComponent*	Target;

	/**
	* Abstract function to execute the tick.
	* @param DeltaTime - frame time to advance, in seconds.
	* @param TickType - kind of tick for this frame.
	* @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created.
	* @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completetion of this task until certain child tasks are complete.
	*/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph. */
	virtual FString DiagnosticMessage() override;
};


/**
 * SkeletalMeshComponent is used to create an instance of an animated SkeletalMesh asset.
 *
 * @see https://docs.unrealengine.com/latest/INT/Engine/Content/Types/SkeletalMeshes/
 * @see USkeletalMesh
 */
UCLASS(ClassGroup=(Rendering, Common), hidecategories=Object, config=Engine, editinlinenew, meta=(BlueprintSpawnableComponent))
class ENGINE_API USkeletalMeshComponent : public USkinnedMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()
	
	/**
	 * Animation 
	 */
	
	/** @Todo anim: Matinee related data start - this needs to be replaced to new system. */
	
	/** @Todo anim: Matinee related data end - this needs to be replaced to new system. */

protected:
	/** Whether to use Animation Blueprint or play Single Animation Asset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
	TEnumAsByte<EAnimationMode::Type>	AnimationMode;

public:
#if WITH_EDITORONLY_DATA
	/** The blueprint for creating an AnimationScript. */
	UPROPERTY()
	class UAnimBlueprint* AnimationBlueprint_DEPRECATED;
#endif

	/* The AnimBlueprint class to use. Use 'SetAnimInstanceClass' to change at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	class UAnimBlueprintGeneratedClass* AnimBlueprintGeneratedClass;

	/** The active animation graph program instance. */
	UPROPERTY(transient)
	class UAnimInstance* AnimScriptInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation, meta=(ShowOnlyInnerProperties))
	struct FSingleAnimationPlayData AnimationData;

	/** Temporary array of local-space (relative to parent bone) rotation/translation for each bone. */
	TArray<FTransform> LocalAtoms;
	
	// Update Rate

	/** Cached LocalAtoms for Update Rate optimization. */
	UPROPERTY(Transient)
	TArray<FTransform> CachedLocalAtoms;

	/** Cached SpaceBases for Update Rate optimization. */
	UPROPERTY(Transient)
	TArray<FTransform> CachedSpaceBases;

	/** Cached Curve result for Update Rate optimization */
	FBlendedCurve CachedCurve;

	/** Used to scale speed of all animations on this skeletal mesh. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category=Animation)
	float GlobalAnimRateScale;

	/** If true, there is at least one body in the current PhysicsAsset with a valid bone in the current SkeletalMesh */
	UPROPERTY(transient)
	uint32 bHasValidBodies:1;

	/** Set during InitArticulated, to indicate if there are bodies in the sync scene */
	uint32 bHasBodiesInSyncScene:1;

	/** Set during InitArticulated, to indicate if there are bodies in the async scene */
	uint32 bHasBodiesInAsyncScene:1;

	/** If we are running physics, should we update non-simulated bones based on the animation bone positions. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category=SkeletalMesh)
	TEnumAsByte<EKinematicBonesUpdateToPhysics::Type> KinematicBonesUpdateType;

	/** Enables blending in of physics bodies whether Simulate or not*/
	UPROPERTY(transient)
	uint32 bBlendPhysics:1;

	/**
	 *  If true, simulate physics for this component on a dedicated server.
	 *  This should be set if simulating physics and replicating with a dedicated server.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = SkeletalMesh)
	uint32 bEnablePhysicsOnDedicatedServer:1;

	/**
	 *	If we should pass joint position to joints each frame, so that they can be used by motorized joints to drive the
	 *	ragdoll based on the animation.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category=SkeletalMesh)
	uint32 bUpdateJointsFromAnimation:1;

	/** Disable cloth simulation and play original animation without simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Clothing)
	uint32 bDisableClothSimulation:1;

	/** can't collide with part of environment if total collision volumes exceed 16 capsules or 32 planes per convex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Clothing)
	uint32 bCollideWithEnvironment:1;
	/** can't collide with part of attached children if total collision volumes exceed 16 capsules or 32 planes per convex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Clothing)
	uint32 bCollideWithAttachedChildren:1;
	/**
	 * It's worth trying this option when you feel that the current cloth simulation is unstable.
	 * The scale of the actor is maintained during the simulation. 
	 * It is possible to add the inertia effects to the simulation, through the inertiaScale parameter of the clothing material. 
	 * So with an inertiaScale of 1.0 there should be no visible difference between local space and global space simulation. 
	 * Known issues: - Currently there's simulation issues when this feature is used in 3.x (DE4076) So if localSpaceSim is enabled there's no inertia effect when the global pose of the clothing actor changes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Clothing)
	uint32 bLocalSpaceSimulation : 1;

	/**
	 * cloth morph target option
	 * This option will be applied only before playing because should do pre-calculation to reduce computation time for run-time play
	 * so it's impossible to change this option in run-time
	 */
	UPROPERTY(EditAnywhere, Category = Clothing)
	uint32 bClothMorphTarget : 1;

	/** reset the clothing after moving the clothing position (called teleport) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Clothing)
	uint32 bResetAfterTeleport:1;
	/** 
	 * conduct teleportation if the character's movement is greater than this threshold in 1 frame. 
	 * Zero or negative values will skip the check 
	 * you can also do force teleport manually using ForceNextUpdateTeleport() / ForceNextUpdateTeleportAndReset()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Clothing)
	float TeleportDistanceThreshold;
	/** 
	 * rotation threshold in degree, ranging from 0 to 180
	 * conduct teleportation if the character's rotation is greater than this threshold in 1 frame. 
	 * Zero or negative values will skip the check 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Clothing)
	float TeleportRotationThreshold;

	/**
	 * weight to blend between simulated results and key-framed positions
	 * if weight is 1.0, shows only cloth simulation results and 0.0 will show only skinned results
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Clothing)
	float ClothBlendWeight;

	/** Draw the APEX Clothing Normals on clothing sections. */
	uint32 bDisplayClothingNormals:1;

	/** Draw Computed Normal Vectors in Tangent space for clothing section */
	uint32 bDisplayClothingTangents:1;

	/** 
	 * Draw Collision Volumes from apex clothing asset. 
	 * Supports up to 16 capsules / 32 planes per convex and ignored collisions by a max number will be drawn in Dark Gray.
	 */
	uint32 bDisplayClothingCollisionVolumes:1;
	
	/** Draw clothing physical mesh wire frame */
	uint32 	bDisplayClothPhysicalMeshWire:1;	

	/** Draw max distances of clothing simulation vertices */
	uint32 	bDisplayClothMaxDistances:1;

	/** Draw back stops of clothing simulation vertices */
	uint32 	bDisplayClothBackstops:1;

	/** To save previous state */
	uint32 bPrevDisableClothSimulation:1;

	uint32 bDisplayClothFixedVertices:1;
	/**
	 * Vertex Animation
	 */
	
	/** Offset of the root bone from the reference pose. Used to offset bounding box. */
	UPROPERTY(transient)
	FVector RootBoneTranslation;

	/** 
	 * Optimization
	 */
	
	/** Skips Ticking and Bone Refresh. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category=SkeletalMesh)
	uint32 bNoSkeletonUpdate:1;

	/** pauses this component's animations (doesn't tick them, but still refreshes bones) */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category=Animation)
	uint32 bPauseAnims:1;

	/**
	* Uses skinned data for collision data.
	*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadOnly, Category=SkeletalMesh)
	uint32 bEnablePerPolyCollision:1;

	/**
	* Used for per poly collision. In 99% of cases you will be better off using a Physics Asset.
	* This BodySetup is per instance because all modification of vertices is done in place */
	UPROPERTY(transient)
	class UBodySetup * BodySetup;

	void CreateBodySetup();

	/**
	 * Misc 
	 */
	
	/** If true TickPose() will not be called from the Component's TickComponent function.
	* It will instead be called from Autonomous networking updates. See ACharacter. */
	UPROPERTY(Transient)
	uint32 bAutonomousTickPose : 1;

	/** If true, force the mesh into the reference pose - is an optimization. */
	UPROPERTY()
	uint32 bForceRefpose:1;

	/** If bForceRefPose was set last tick. */
	UPROPERTY()
	uint32 bOldForceRefPose:1;

	/** Bool that enables debug drawing of the skeleton before it is passed to the physics. Useful for debugging animation-driven physics. */
	UPROPERTY()
	uint32 bShowPrePhysBones:1;

	/** If false, indicates that on the next call to UpdateSkelPose the RequiredBones array should be recalculated. */
	UPROPERTY(transient)
	uint32 bRequiredBonesUpToDate:1;

	/** If true, AnimTree has been initialised. */
	UPROPERTY(transient)
	uint32 bAnimTreeInitialised:1;

	/** If true, line checks will test against the bounding box of this skeletal mesh component and return a hit if there is a collision. */
	UPROPERTY()
	uint32 bEnableLineCheckWithBounds:1;

	/** If bEnableLineCheckWithBounds is true, scale the bounds by this value before doing line check. */
	UPROPERTY()
	FVector LineCheckBoundsScale;

	/** Threshold for physics asset bodies above which we use an aggregate for broadphase collisions */
	UPROPERTY()
	int32 RagdollAggregateThreshold;

	/** Notification when constraint is broken. */
	UPROPERTY(BlueprintAssignable)
	FConstraintBrokenSignature OnConstraintBroken;

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh", meta=(Keywords = "AnimBlueprint"))
	void SetAnimInstanceClass(class UClass* NewClass);

	/** 
	 * Returns the animation instance that is driving the class (if available). This is typically an instance of
	 * the class set as AnimBlueprintGeneratedClass (generated by an animation blueprint)
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh", meta=(Keywords = "AnimBlueprint"))
	class UAnimInstance * GetAnimInstance() const;

	/** Below are the interface to control animation when animation mode, not blueprint mode **/
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetAnimationMode(EAnimationMode::Type InAnimationMode);
	
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	EAnimationMode::Type GetAnimationMode() const;

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void PlayAnimation(class UAnimationAsset* NewAnimToPlay, bool bLooping);

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetAnimation(class UAnimationAsset* NewAnimToPlay);

	// @todo block this until we support vertex animation 
//	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetVertexAnimation(class UVertexAnimation* NewVertexAnimToPlay);

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void Play(bool bLooping);

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void Stop();

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	bool IsPlaying() const;

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetPosition(float InPos, bool bFireNotifies = true);

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	float GetPosition() const;

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetPlayRate(float Rate);

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	float GetPlayRate() const;

	/**
	 * Set Morph Target with Name and Value(0-1)
	 *
	 * @param bRemoveZeroWeight : Used by editor code when it should stay in the active list with zero weight
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh", meta=(UnsafeDuringActorConstruction="true"))
	void SetMorphTarget(FName MorphTargetName, float Value, bool bRemoveZeroWeight=true);

	/**
	 * Clear all Morph Target that are set to this mesh
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void ClearMorphTargets();

	/**
	 * Get Morph target with given name
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	float GetMorphTarget(FName MorphTargetName) const;

	/**
	 * Get/Set the max distance scale of clothing mesh vertices
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	float GetClothMaxDistanceScale();
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetClothMaxDistanceScale(float Scale);

	/** 
	 * Used to indicate we should force 'teleport' during the next call to UpdateClothState, 
	 * This will transform positions and velocities and thus keep the simulation state, just translate it to a new pose.
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void ForceClothNextUpdateTeleport();
	/** 
	 * Used to indicate we should force 'teleport and reset' during the next call to UpdateClothState.
	 * This can be used to reset it from a bad state or by a teleport where the old state is not important anymore.
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void ForceClothNextUpdateTeleportAndReset();

	/**
	 * Reset the teleport mode of a next update to 'Continuous'
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void ResetClothTeleportMode();

	/** 
	 * If this component has a valid MasterPoseComponent then this function makes cloth items on the slave component
	 * take the transforms of the cloth items on the master component instead of simulating separately.
	 * @Note This will FORCE any cloth actor on the master component to simulate in local space. Also
	 * The meshes used in the components must be identical for the cloth to bind correctly
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh", meta=(UnsafeDuringActorConstruction="true"))
	void BindClothToMasterPoseComponent();

	/** 
	 * If this component has a valid MasterPoseComponent and has previously had its cloth bound to the
	 * MCP, this function will unbind the cloth and resume simulation.
	 * @param bRestoreSimulationSpace if true and the master pose cloth was originally simulating in world
	 * space, we will restore this setting. This will cause the master component to reset which may be
	 * undesirable.
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh", meta=(UnsafeDuringActorConstruction="true"))
	void UnbindClothFromMasterPoseComponent(bool bRestoreSimulationSpace = true);

	/** We detach the Component once we are done playing it.
	 *
	 * @param	ParticleSystemComponent that finished
	 */
	virtual void SkelMeshCompOnParticleSystemFinished( class UParticleSystemComponent* PSC );

	class UAnimSingleNodeInstance * GetSingleNodeInstance() const;
	void InitializeAnimScriptInstance(bool bForceReinit=true);

	/** @return true if wind is enabled */
	virtual bool IsWindEnabled() const;

#if WITH_EDITOR
	/**
	* Subclasses such as DebugSkelMeshComponent keep track of errors in the anim notifies so they can be displayed to the user. This function adds an error.
	* Errors are added uniquely and only removed when they're cleared by ClearAnimNotifyError.
	*/
	virtual void ReportAnimNotifyError(const FText& Error, UObject* InSourceNotify){}
	
	/**
	* Clears currently stored errors. Call before triggering anim notifies for a particular mesh.
	*/
	virtual void ClearAnimNotifyErrors(UObject* InSourceNotify){}
#endif

public:
	/** Temporary array of bone indices required this frame. Filled in by UpdateSkelPose. */
	TArray<FBoneIndexType> RequiredBones;

	/** Tempory array of bone indices required to populate space bases */
	TArray<FBoneIndexType> FillSpaceBasesRequiredBones;

	/** 
	 *	Index of the 'Root Body', or top body in the asset hierarchy. 
	 *	Filled in by InitInstance, so we don't need to save it.
	 */
	/** To save root body index/bone index consistently **/
	struct 
	{
		int32 BodyIndex;
		FTransform TransformToRoot;
	} RootBodyData;

	/** Set Root Body Index */
	void SetRootBodyIndex(int32 InBodyIndex);

	/** Array of FBodyInstance objects, storing per-instance state about about each body. */
	TArray<struct FBodyInstance*> Bodies;

	/** Array of FConstraintInstance structs, storing per-instance state about each constraint. */
	TArray<struct FConstraintInstance*> Constraints;

#if WITH_PHYSX
	/** Physics-engine representation of PxAggregate which contains a physics asset instance with more than numbers of bodies. */
	class physx::PxAggregate* Aggregate;

#endif	//WITH_PHYSX

	FSkeletalMeshComponentPreClothTickFunction PreClothTickFunction;

#if WITH_APEX_CLOTHING
	/** 
	* clothing actors will be created from clothing assets for cloth simulation 
	* 1 actor should correspond to 1 asset
	*/
	TArray<FClothingActor> ClothingActors;

	float ClothMaxDistanceScale;

	FClothingActor::TeleportMode ClothTeleportMode;
	/** previous root bone matrix to compare the difference and decide to do clothing teleport  */
	FMatrix	PrevRootBoneMatrix;
	/** used for pre-computation using TeleportRotationThreshold property */
	float ClothTeleportCosineThresholdInRad;
	/** used for pre-computation using tTeleportDistanceThreshold property */
	float ClothTeleportDistThresholdSquared;
	/** 
	 * clothing reset is needed once more to avoid clothing pop up 
	 * use until Apex clothing bug is resolved 
	 */
	bool bNeedTeleportAndResetOnceMore;
	/** used for checking whether cloth morph target data were pre-computed or not */
	bool bPreparedClothMorphTargets;
	/** precomputed actual cloth morph target data */
	TArray<FClothMorphTargetData> ClothMorphTargets;

	/** Whether or not we're taking cloth sim information from our master component */
	bool bBindClothToMasterComponent;
	/** The previous state of the master component simulation coord space, so we can restore on unbind */
	bool bPrevMasterSimulateLocalSpace;

#if WITH_CLOTH_COLLISION_DETECTION
	/** increase every tick to update clothing collision  */
	uint32 ClothingCollisionRevision; 

	TArray<physx::apex::NxClothingCollision*>	ParentCollisions;
	TArray<physx::apex::NxClothingCollision*>	EnvironmentCollisions;
	TArray<physx::apex::NxClothingCollision*>	ChildrenCollisions;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, FApexClothCollisionInfo> ClothOverlappedComponentsMap;
#endif // WITH_CLOTH_COLLISION_DETECTION

#endif // WITH_APEX_CLOTHING

	/** 
	 * Morph Target Curves. This will override AnimInstance MorphTargetCurves
	 * if same curve is found
	 **/
	TMap<FName, float>	MorphTargetCurves;

	// 
	// Animation
	//
	virtual void InitAnim(bool bForceReinit);

	/** Tick Animation system */
	void TickAnimation(float DeltaTime);

	/** Tick Clothing Animation , basically this is called inside TickComponent */
	void TickClothing(float DeltaTime);

	/** Store cloth simulation data into OutClothSimData */
	void GetUpdateClothSimulationData(TArray<FClothSimulData>& OutClothSimData, USkeletalMeshComponent* OverrideLocalRootComponent = nullptr);
	void ApplyWindForCloth(FClothingActor& ClothingActor);
	void RemoveAllClothingActors();
	void ReleaseAllClothingResources();

	bool IsValidClothingActor(int32 ActorIndex) const;
	/** Draws APEX Clothing simulated normals on cloth meshes **/
	void DrawClothingNormals(FPrimitiveDrawInterface* PDI);
	/** Draws APEX Clothing Graphical Tangents on cloth meshes **/
	void DrawClothingTangents(FPrimitiveDrawInterface* PDI);
	/** Draws internal collision volumes which the character has, colliding with cloth **/
	void DrawClothingCollisionVolumes(FPrimitiveDrawInterface* PDI);
	/** Draws max distances of clothing simulation vertices 
	  * clothing simulation will be disabled and animation will be reset when drawing this option 
	  * because max distances do have meaning only in initial pose
	 **/
	void DrawClothingMaxDistances(FPrimitiveDrawInterface* PDI);
	/** Draws Clothing back stops **/
	void DrawClothingBackstops(FPrimitiveDrawInterface* PDI);
	/** Draws Clothing Physical mesh wire **/
	void DrawClothingPhysicalMeshWire(FPrimitiveDrawInterface* PDI);

	void DrawClothingFixedVertices(FPrimitiveDrawInterface* PDI);

	/** Loads clothing extra infos dynamically just for Previewing in Editor 
	 *  such as MaxDistances, Physical mesh wire
	 **/
	void LoadClothingVisualizationInfo(int32 AssetIndex);
	void LoadAllClothingVisualizationInfos();

	/** freezing clothing actor now */
	void FreezeClothSection(bool bFreeze);

	/** 
	 * Recalculates the RequiredBones array in this SkeletalMeshComponent based on current SkeletalMesh, LOD and PhysicsAsset.
	 * Is called when bRequiredBonesUpToDate = false
	 *
	 * @param	LODIndex	Index of LOD [0-(MaxLOD-1)]
	 */
	void RecalcRequiredBones(int32 LODIndex);

public:
	// Begin UObject interface.
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE(FOnSkeletalMeshPropertyChangedMulticaster)
	FOnSkeletalMeshPropertyChangedMulticaster OnSkeletalMeshPropertyChanged;
	typedef FOnSkeletalMeshPropertyChangedMulticaster::FDelegate FOnSkeletalMeshPropertyChanged;

	/** Register / Unregister delegates called when the skeletal mesh property is changed */
	FDelegateHandle RegisterOnSkeletalMeshPropertyChanged(const FOnSkeletalMeshPropertyChanged& Delegate);
	DELEGATE_DEPRECATED("This overload of UnregisterOnSkeletalMeshPropertyChanged is deprecated, instead pass the result of RegisterOnSkeletalMeshPropertyChanged.")
	void UnregisterOnSkeletalMeshPropertyChanged(const FOnSkeletalMeshPropertyChanged& Delegate);
	void UnregisterOnSkeletalMeshPropertyChanged(FDelegateHandle Handle);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Validates the animation asset or blueprint, making sure it is compatible with the current skeleton */
	void ValidateAnimation();

	virtual void LoadedFromAnotherClass(const FName& OldClassName) override;
	virtual void UpdateCollisionProfile() override;
#endif // WITH_EDITOR
	virtual SIZE_T GetResourceSize(EResourceSizeMode::Type Mode) override;
	// End UObject interface.

	// Begin UActorComponent interface.
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void CreateRenderState_Concurrent() override;
	virtual void CreatePhysicsState() override;
	virtual void DestroyPhysicsState() override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	//Handle registering our pre cloth tick function
	void RegisterPreClothTick(bool bRegister);

	// End UActorComponent interface.

	// Begin USceneComponent interface.
	virtual void UpdateBounds() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual bool IsAnySimulatingPhysics() const override;
	virtual void OnUpdateTransform(bool bSkipPhysicsMove, ETeleportType Teleport = ETeleportType::None) override;
	virtual void UpdateOverlaps(TArray<FOverlapInfo> const* PendingOverlaps=NULL, bool bDoNotifies=true, const TArray<FOverlapInfo>* OverlapsAtEndLocation=NULL) override;
	// End USceneComponent interface.

	// Begin UPrimitiveComponent interface.
protected:
	/**
	 *  Test the collision of the supplied component at the supplied location/rotation, and determine the set of components that it overlaps
	 *  @param  OutOverlaps     Array of overlaps found between this component in specified pose and the world
	 *  @param  World			World to use for overlap test
	 *  @param  Pos             Location of the component's geometry for the test against the world
	 *  @param  Rot             Rotation of the component's geometry for the test against the world
	 *  @param  TestChannel		The 'channel' that this ray is in, used to determine which components to hit
	 *	@param	ObjectQueryParams	List of object types it's looking for. When this enters, we do object query with component shape
	 *  @return TRUE if OutOverlaps contains any blocking results
	 */
	virtual bool ComponentOverlapMultiImpl(TArray<struct FOverlapResult>& OutOverlaps, const class UWorld* World, const FVector& Pos, const FQuat& Rot, ECollisionChannel TestChannel, const struct FComponentQueryParams& Params, const struct FCollisionObjectQueryParams& ObjectQueryParams = FCollisionObjectQueryParams::DefaultObjectQueryParam) const override;
	
	virtual bool ComponentOverlapComponentImpl(class UPrimitiveComponent* PrimComp, const FVector Pos, const FQuat& Quat, const FCollisionQueryParams& Params) override;

public:

	virtual class UBodySetup* GetBodySetup() override;
	virtual bool CanEditSimulatePhysics() override;
	virtual FBodyInstance* GetBodyInstance(FName BoneName = NAME_None, bool bGetWelded = true) const override;
	virtual void UpdatePhysicsToRBChannels() override;
	virtual void SetAllPhysicsAngularVelocity(FVector const& NewVel, bool bAddToCurrent = false) override;
	virtual void SetAllPhysicsPosition(FVector NewPos) override;
	virtual void SetAllPhysicsRotation(FRotator NewRot) override;
	virtual void WakeAllRigidBodies() override;
	virtual void PutAllRigidBodiesToSleep() override;
	virtual bool IsAnyRigidBodyAwake() override;
	virtual void OnComponentCollisionSettingsChanged() override;
	virtual void SetPhysMaterialOverride(UPhysicalMaterial* NewPhysMaterial) override;
	virtual float GetDistanceToCollision(const FVector& Point, FVector& ClosestPointOnCollision) const override;
	virtual bool LineTraceComponent( FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionQueryParams& Params ) override;
	virtual bool SweepComponent( FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionShape& CollisionShape, bool bTraceComplex=false) override;
	virtual bool OverlapComponent(const FVector& Pos, const FQuat& Rot, const FCollisionShape& CollisionShape) override;
	virtual void SetSimulatePhysics(bool bEnabled) override;
	virtual void AddRadialImpulse(FVector Origin, float Radius, float Strength, ERadialImpulseFalloff Falloff, bool bVelChange=false) override;
	virtual void AddRadialForce(FVector Origin, float Radius, float Strength, ERadialImpulseFalloff Falloff, bool bAccelChange=false) override;
	virtual void SetAllPhysicsLinearVelocity(FVector NewVel,bool bAddToCurrent = false) override;
	virtual void SetAllMassScale(float InMassScale = 1.f) override;
	virtual float GetMass() const override;
	virtual float CalculateMass(FName BoneName = NAME_None) override;
	virtual bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const override;
#if WITH_EDITOR
	virtual bool ComponentIsTouchingSelectionBox(const FBox& InSelBBox, const FEngineShowFlags& ShowFlags, const bool bConsiderOnlyBSP, const bool bMustEncompassEntireComponent) const override;
	virtual bool ComponentIsTouchingSelectionFrustum(const FConvexVolume& InFrustum, const FEngineShowFlags& ShowFlags, const bool bConsiderOnlyBSP, const bool bMustEncompassEntireComponent) const override;
#endif
protected:
	virtual FTransform GetComponentTransformFromBodyInstance(FBodyInstance* UseBI) override;
	// End UPrimitiveComponent interface.

public:
	// Begin USkinnedMeshComponent interface
	virtual bool UpdateLODStatus() override;
	virtual void RefreshBoneTransforms( FActorComponentTickFunction* TickFunction = NULL ) override;
	virtual void TickPose(float DeltaTime, bool bNeedsValidRootMotion) override;
	virtual void UpdateSlaveComponent() override;
	virtual bool ShouldUpdateTransform(bool bLODHasChanged) const override;
	virtual bool ShouldTickPose() const override;
	virtual bool AllocateTransformData() override;
	virtual void DeallocateTransformData() override;
	virtual void HideBone( int32 BoneIndex, EPhysBodyOp PhysBodyOption ) override;
	virtual void UnHideBone( int32 BoneIndex ) override;
	virtual void SetPhysicsAsset(class UPhysicsAsset* NewPhysicsAsset,bool bForceReInit = false) override;
	virtual void SetSkeletalMesh(class USkeletalMesh* NewMesh) override;
	virtual FVector GetSkinnedVertexPosition(int32 VertexIndex) const override;

	void SetSkeletalMeshWithoutResettingAnimation(class USkeletalMesh* NewMesh);

	virtual bool IsPlayingRootMotion() override;
	virtual bool IsPlayingRootMotionFromEverything() override;

	// End USkinnedMeshComponent interface
	/** 
	 *	Iterate over each joint in the physics for this mesh, setting its AngularPositionTarget based on the animation information.
	 */
	void UpdateRBJointMotors();


	/**
	* Runs the animation evaluation for the current pose into the supplied variables
	*
	* @param	InSkeletalMesh			The skeletal mesh we are animating
	* @param	InAnimInstance			The anim instance we are evaluating
	* @param	OutSpaceBases			Component space bone transforms
	* @param	OutLocalAtoms			Local space bone transforms
	* @param	OutVertexAnims			Active vertex animations
	* @param	OutRootBoneTranslation	Calculated root bone translation
	*/
	void PerformAnimationEvaluation(const USkeletalMesh* InSkeletalMesh, UAnimInstance* InAnimInstance, TArray<FTransform>& OutSpaceBases, TArray<FTransform>& OutLocalAtoms, TArray<FActiveVertexAnim>& OutVertexAnims, FVector& OutRootBoneTranslation, FBlendedCurve& OutCurve) const;
	void PostAnimEvaluation( FAnimationEvaluationContext& EvaluationContext );

	/**
	 * Blend of Physics Bones with PhysicsWeight and Animated Bones with (1-PhysicsWeight)
	 *
	 * @param	RequiredBones	List of bones to be blend
	 */
	void BlendPhysicsBones( TArray<FBoneIndexType>& Bones )
	{
		PerformBlendPhysicsBones(Bones, AnimEvaluationContext.LocalAtoms);
	}


	/** Take the results of the physics and blend them with the animation state (based on the PhysicsWeight parameter), and update the SpaceBases array. */
	void BlendInPhysics();	

	/** 
	 * Intialize PhysicsAssetInstance for the physicsAsset 
	 * 
	 * @param	PhysScene	Physics Scene
	 */
	void InitArticulated(FPhysScene* PhysScene);

	/** Turn off all physics and remove the instance. */
	void TermArticulated();


	/** Terminate physics on all bodies below the named bone */
	void TermBodiesBelow(FName ParentBoneName);

	/** Find instance of the constraint that matches the name supplied. */
	FConstraintInstance* FindConstraintInstance(FName ConName);

	/** Utility which returns total mass of all bones below the supplied one in the hierarchy (including this one). */
	float GetTotalMassBelowBone(FName InBoneName);

	/** Set the movement channel of all bodies */
	void SetAllBodiesCollisionObjectType(ECollisionChannel NewChannel);

	/** Set the rigid body notification state for all bodies. */
	void SetAllBodiesNotifyRigidBodyCollision(bool bNewNotifyRigidBodyCollision);

	/** Set bSimulatePhysics to true for all bone bodies. Does not change the component bSimulatePhysics flag. */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetAllBodiesSimulatePhysics(bool bNewSimulate);

	/** Update Material Parameters based on AnimInstance */
	void UpdateMaterialParameters();

	/** This is global set up for setting physics blend weight
	 * This does multiple things automatically
	 * If PhysicsBlendWeight == 1.f, it will enable Simulation, and if PhysicsBlendWeight == 0.f, it will disable Simulation. 
	 * Also it will respect each body's setup, so if the body is fixed, it won't simulate. Vice versa
	 * So if you'd like all bodies to change manually, do not use this function, but SetAllBodiesPhysicsBlendWeight
	 */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetPhysicsBlendWeight(float PhysicsBlendWeight);

	/** Disable physics blending of bones **/
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetEnablePhysicsBlending(bool bNewBlendPhysics);

	/** Set all of the bones below passed in bone to be simulated */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetAllBodiesBelowSimulatePhysics(const FName& InBoneName, bool bNewSimulate );

	/** Allows you to reset bodies Simulate state based on where bUsePhysics is set to true in the BodySetup. */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void ResetAllBodiesSimulatePhysics();

	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetAllBodiesPhysicsBlendWeight(float PhysicsBlendWeight, bool bSkipCustomPhysicsType = false );

	/** Set all of the bones below passed in bone to be simulated */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void SetAllBodiesBelowPhysicsBlendWeight(const FName& InBoneName, float PhysicsBlendWeight, bool bSkipCustomPhysicsType = false );

	/** Accumulate AddPhysicsBlendWeight to physics blendweight for all of the bones below passed in bone to be simulated */
	UFUNCTION(BlueprintCallable, Category="Components|SkeletalMesh")
	void AccumulateAllBodiesBelowPhysicsBlendWeight(const FName& InBoneName, float AddPhysicsBlendWeight, bool bSkipCustomPhysicsType = false );

	/** Enable or Disable AngularPositionDrive */
	void SetAllMotorsAngularPositionDrive(bool bEnableSwingDrive, bool bEnableTwistDrive, bool bSkipCustomPhysicsType = false);

	/** Enable or Disable AngularVelocityDrive based on a list of bone names */
	void SetAllMotorsAngularVelocityDrive(bool bEnableSwingDrive, bool bEnableTwistDrive, bool bSkipCustomPhysicsType = false);

	/** Set Angular Drive motors params for all constraint instance */
	void SetAllMotorsAngularDriveParams(float InSpring, float InDamping, float InForceLimit, bool bSkipCustomPhysicsType = false);

	/** Enable or Disable AngularPositionDrive based on a list of bone names */
	void SetNamedMotorsAngularPositionDrive(bool bEnableSwingDrive, bool bEnableTwistDrive, const TArray<FName>& BoneNames, bool bSetOtherBodiesToComplement = false);

	/** Enable or Disable AngularVelocityDrive based on a list of bone names */
	void SetNamedMotorsAngularVelocityDrive(bool bEnableSwingDrive, bool bEnableTwistDrive, const TArray<FName>& BoneNames, bool bSetOtherBodiesToComplement = false);

	void GetWeldedBodies(TArray<FBodyInstance*> & OutWeldedBodies, TArray<FName> & OutChildrenLabels) override;

	/**
	 * Return Transform Matrix for SkeletalMeshComponent considering root motion setups
	 * 
	 * @return Matrix Transform matrix
	 */
	FMatrix GetTransformMatrix() const;
	
	/** 
	 * Change whether to force mesh into ref pose (and use cheaper vertex shader) 
	 *
	 * @param	bNewForceRefPose	true if it would like to force ref pose
	 */
	void SetForceRefPose(bool bNewForceRefPose);
	
	/** Update bHasValidBodies flag */
	void UpdateHasValidBodies();
	
	/** 
	 * Initialize SkelControls
	 */
	void InitSkelControls();
	
	/**
	 * Find Constraint Index from the name
	 * 
	 * @param	ConstraintName	Joint Name of constraint to look for
	 * @return	Constraint Index
	 */
	int32	FindConstraintIndex(FName ConstraintName);
	
	/**
	 * Find Constraint Name from index
	 * 
	 * @param	ConstraintIndex	Index of constraint to look for
	 * @return	Constraint Joint Name
	 */
	FName	FindConstraintBoneName(int32 ConstraintIndex);

	/** 
	 *	Iterate over each physics body in the physics for this mesh, and for each 'kinematic' (ie fixed or default if owner isn't simulating) one, update its
	 *	transform based on the animated transform.
	 */
	void UpdateKinematicBonesToAnim(const TArray<FTransform>& InSpaceBases, ETeleportType Teleport, bool bNeedsSkinning);

	DEPRECATED(4.9, "bForceUpdate is no longer used. Please use Teleport")
	void UpdateKinematicBonesToAnim(const TArray<FTransform>& InSpaceBases, bool bTeleport, bool bNeedsSkinning, bool bForceUpdate)
	{
		UpdateKinematicBonesToAnim(InSpaceBases, TeleportFlagToEnum(bTeleport || bForceUpdate), bNeedsSkinning);
	}
	
	/**
	 * Look up all bodies for broken constraints.
	 * Makes sure child bodies of a broken constraints are not fixed and using bone springs, and child joints not motorized.
	 */
	void UpdateMeshForBrokenConstraints();
	
	/**
	 * Notifier when look at control goes beyond of limit - candidate for delegate
	 */
	virtual void NotifySkelControlBeyondLimit(class USkelControlLookAt* LookAt);

	/** 
	 * Break a constraint off a Gore mesh. 
	 * 
	 * @param	Impulse	vector of impulse
	 * @param	HitLocation	location of the hit
	 * @param	InBoneName	Name of bone to break constraint for
	 */
	void BreakConstraint(FVector Impulse, FVector HitLocation, FName InBoneName);
	
	/** iterates through all bodies in our PhysicsAsset and returns the location of the closest bone associated
	 * with a body that has collision enabled.
	 * @param TestLocation - location to check against
	 * @return location of closest colliding rigidbody, or TestLocation if there were no bodies to test
	 */
	FVector GetClosestCollidingRigidBodyLocation(const FVector& TestLocation) const;

	/** Calls needed cloth updates */
	void PreClothTick(float DeltaTime);
	
	/** Set physics transforms for all bodies */
	void ApplyDeltaToAllPhysicsTransforms(const FVector& DeltaLocation, const FQuat& DeltaRotation);

#if WITH_APEX_CLOTHING
	/** 
	* APEX clothing actor is created from APEX clothing asset for cloth simulation 
	* create only if became invalid
	* BlendedData : added for cloth morph target but not used commonly
	*/
	bool CreateClothingActor(int32 AssetIndex, physx::apex::NxClothingAsset* ClothingAsset, TArray<FVector>* BlendedDelta = NULL);
	/** should call this method if occurred any changes in clothing assets */
	void ValidateClothingActors();
	/** add bounding box for cloth */
	void AddClothingBounds(FBoxSphereBounds& InOutBounds) const;
	/** changes clothing LODs, if clothing LOD is disabled or LODIndex is greater than apex clothing LODs, simulation will be disabled */
	void SetClothingLOD(int32 LODIndex);
	/** check whether clothing teleport is needed or not to avoid a weird simulation result */
	virtual void CheckClothTeleport(float DeltaTime);

	/** 
	* methods for cloth morph targets 
	*/
	/** pre-compute morph target data for clothing */
	void PrepareClothMorphTargets();
	/** change morph target mapping when active morph target is changed */
	void ChangeClothMorphTargetMapping(FClothMorphTargetData& MorphData, FName CurrentActivatedMorphName);
	/** update active morph target's blending data when morph weight is changed */
	void UpdateClothMorphTarget();

	/** 
	 * Updates all clothing animation states including ComponentToWorld-related states.
	 */
	void UpdateClothState(float DeltaTime);
	/** 
	 * Updates clothing actor's global pose.
	 * So should be called when ComponentToWorld is changed.
	 */
	void UpdateClothTransform();

	/** only check whether there are valid clothing actors or not */
	bool HasValidClothingActors();

	/** get root bone matrix by the root bone index which Apex cloth asset is holding */
	void GetClothRootBoneMatrix(int32 AssetIndex, FMatrix& OutRootBoneMatrix) const;

	/** if the vertex index is valid for simulated vertices, returns the position in world space */
	bool GetClothSimulatedPosition(int32 AssetIndex, int32 VertexIndex, FVector& OutSimulPos) const;

#if WITH_CLOTH_COLLISION_DETECTION

	/** draws currently intersected collisions */
	void DrawDebugClothCollisions();
	/** draws a convex from planes for debug info */
	void DrawDebugConvexFromPlanes(FClothCollisionPrimitive& CollisionPrimitive, FColor& Color, bool bDrawWithPlanes=true);
	void ReleaseClothingCollision(physx::apex::NxClothingCollision* Collision);
	/** create new collisions when newly added  */
	FApexClothCollisionInfo* CreateNewClothingCollsions(UPrimitiveComponent* PrimitiveComponent);

	void RemoveAllOverlappedComponentMap();
	/** for non-static collisions which need to be updated every tick */ 
	void UpdateOverlappedComponent(UPrimitiveComponent* PrimComp, FApexClothCollisionInfo* Info);

	void ReleaseAllParentCollisions();
	void ReleaseAllChildrenCollisions();

	void ProcessClothCollisionWithEnvironment();
	/** copy parent's cloth collisions to attached children, where parent means this component */
	void CopyClothCollisionsToChildren();
	/** copy children's cloth collisions to parent, where parent means this component */
	void CopyChildrenClothCollisionsToParent();

	/**
  	 * Get collision data from a static mesh only for collision with clothes.
	 * Returns false when failed to get cloth collision data
	*/
	bool GetClothCollisionDataFromStaticMesh(UPrimitiveComponent* PrimComp, TArray<FClothCollisionPrimitive>& ClothCollisionPrimitives);
	/** find if this component has collisions for clothing and return the results calculated by bone transforms */
	void FindClothCollisions(TArray<FApexClothCollisionVolumeData>& OutCollisions);
	/** create Apex clothing collisions from input collision info and add them to clothing actors */
	void CreateInternalClothCollisions(TArray<FApexClothCollisionVolumeData>& InCollisions, TArray<physx::apex::NxClothingCollision*>& OutCollisions);

#endif // WITH_CLOTH_COLLISION_DETECTION


#endif// #if WITH_APEX_CLOTHING
	bool IsAnimBlueprintInstanced() const;

	/** Debug render skeletons bones to supplied canvas */
	void DebugDrawBones(class UCanvas* Canvas, bool bSimpleBones) const;

protected:
	bool NeedToSpawnAnimScriptInstance(bool bForceInit) const;
	
private:
	/** Evaluate Anim System **/
	void EvaluateAnimation(const USkeletalMesh* InSkeletalMesh, UAnimInstance* InAnimInstance, TArray<FTransform>& OutLocalAtoms, TArray<struct FActiveVertexAnim>& OutVertexAnims, FVector& OutRootBoneTranslation, FBlendedCurve& OutCurve) const;

	/**
	* Take the SourceAtoms array (translation vector, rotation quaternion and scale vector) and update the array of component-space bone transformation matrices (DestSpaceBases).
	* It will work down hierarchy multiplying the component-space transform of the parent by the relative transform of the child.
	* This code also applies any per-bone rotators etc. as part of the composition process
	*/
	void FillSpaceBases(const USkeletalMesh* InSkeletalMesh, const TArray<FTransform>& SourceAtoms, TArray<FTransform>& DestSpaceBases) const;

	void RenderAxisGizmo(const FTransform& Transform, class UCanvas* Canvas) const;

	bool ShouldBlendPhysicsBones() const;
	bool DoAnyPhysicsBodiesHaveWeight() const;

	void ClearAnimScriptInstance();
	virtual void RefreshActiveVertexAnims() override;

	//Data for parallel evaluation of animation
	FAnimationEvaluationContext AnimEvaluationContext;

	// Reference to our current parallel animation evaluation task (if there is one)
	FGraphEventRef				ParallelAnimationEvaluationTask;

	// Reference to our current blend physics task (if there is one)
	FGraphEventRef				ParallelBlendPhysicsCompletionTask;

public:
	// Parallel evaluation wrappers
	void ParallelAnimationEvaluation() { PerformAnimationEvaluation(AnimEvaluationContext.SkeletalMesh, AnimEvaluationContext.AnimInstance, AnimEvaluationContext.SpaceBases, AnimEvaluationContext.LocalAtoms, AnimEvaluationContext.VertexAnims, AnimEvaluationContext.RootBoneTranslation, AnimEvaluationContext.Curve); }
	void CompleteParallelAnimationEvaluation(bool bDoPostAnimEvaluation)
	{
		ParallelAnimationEvaluationTask.SafeRelease(); //We are done with this task now, clean up!

		if (bDoPostAnimEvaluation && (AnimEvaluationContext.AnimInstance == AnimScriptInstance) && (AnimEvaluationContext.SkeletalMesh == SkeletalMesh) && (AnimEvaluationContext.SpaceBases.Num() == GetNumSpaceBases()))
		{
			Exchange(AnimEvaluationContext.SpaceBases, AnimEvaluationContext.bDoInterpolation ? CachedSpaceBases : GetEditableSpaceBases() );
			Exchange(AnimEvaluationContext.LocalAtoms, AnimEvaluationContext.bDoInterpolation ? CachedLocalAtoms : LocalAtoms);
			Exchange(AnimEvaluationContext.VertexAnims, ActiveVertexAnims);
			Exchange(AnimEvaluationContext.RootBoneTranslation, RootBoneTranslation);

			PostAnimEvaluation(AnimEvaluationContext);
		}
		AnimEvaluationContext.Clear();
	}

	bool IsRunningParallelEvaluation(bool bBlockOnTask, bool bPerformPostAnimEvaluation);

	friend class FSkeletalMeshComponentDetails;

private:
	// Returns whether we need to run the Pre Cloth Tick or not
	bool ShouldRunPreClothTick() const;

	// Handles registering/unregistering the pre cloth tick as it is needed
	void UpdatePreClothTickRegisteredState();
	friend class FParallelBlendPhysicsTask;
	
	//wrapper for parallel blend physics
	void ParallelBlendPhysics() { PerformBlendPhysicsBones(RequiredBones, AnimEvaluationContext.LocalAtoms); }

	void PerformBlendPhysicsBones(const TArray<FBoneIndexType>& InRequiredBones, TArray<FTransform>& InLocalAtoms);

	friend class FParallelBlendPhysicsCompletionTask;
	void CompleteParallelBlendPhysics();

	friend class FTickClothingTask;
	void PerformTickClothing(float DeltaTime);

	// these are deprecated variables from removing SingleAnimSkeletalComponent
	// remove if this version goes away : VER_UE4_REMOVE_SINGLENODEINSTANCE
	// deprecated variable to be re-save
	UPROPERTY()
	class UAnimSequence* SequenceToPlay_DEPRECATED;

	// The default sequence to play on this skeletal mesh
	UPROPERTY()
	class UAnimationAsset* AnimToPlay_DEPRECATED;

	// Default setting for looping for SequenceToPlay. This is not current state of looping.
	UPROPERTY()
	uint32 bDefaultLooping_DEPRECATED:1;

	// Default setting for playing for SequenceToPlay. This is not current state of playing.
	UPROPERTY()
	uint32 bDefaultPlaying_DEPRECATED:1;

	// Default setting for position of SequenceToPlay to play. 
	UPROPERTY()
	float DefaultPosition_DEPRECATED;

	// Default setting for playrate of SequenceToPlay to play. 
	UPROPERTY()
	float DefaultPlayRate_DEPRECATED;

public:
	/** Keep track of when animation has been ticked to ensure it is ticked only once per frame. */
	UPROPERTY(Transient)
	float LastPoseTickTime;

	/** Checked whether we have already ticked the pose this frame */
	bool PoseTickedThisFrame() const { return LastPoseTickTime == GetWorld()->TimeSeconds; }

	/** Take extracted RootMotion and convert it from local space to world space. */
	FTransform ConvertLocalRootMotionToWorld(const FTransform& InTransform);

	FRootMotionMovementParams ConsumeRootMotion();
};



