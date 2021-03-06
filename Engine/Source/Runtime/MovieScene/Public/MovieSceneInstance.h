// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

class UMovieSceneTrack;

typedef TMap< TWeakObjectPtr<UMovieSceneTrack>, TSharedPtr<class IMovieSceneTrackInstance> > FMovieSceneInstanceMap;
class FMovieSceneInstance : public TSharedFromThis<FMovieSceneInstance>
{
public:
	MOVIESCENE_API FMovieSceneInstance( UMovieScene& InMovieScene );

	/**
	* Save state of the objects that this movie scene controls.
	*/
	MOVIESCENE_API void SaveState ();

	/**
	* Restore state of the objects that this movie scene controls.
	*/
	MOVIESCENE_API void RestoreState ();

	/**
	 * Updates this movie scene
	 *
	 * @param Position The local playback position
	 * @param Player Movie scene player interface for interaction with runtime data
	 */
	MOVIESCENE_API void Update( float Position, float LastPosition, class IMovieScenePlayer& Player );

	/**
	 * Refreshes the existing instance.  Called when something significant about moviescene data changes (like adding or removing a track)
	 * Instances all new tracks found and removes instances for tracks that no longer exist
	 */
	MOVIESCENE_API void RefreshInstance( class IMovieScenePlayer& Player );

	/** @return The movie scene being instanced */
	UMovieScene* GetMovieScene() { return MovieScene.Get(); }
private:
	void RefreshInstanceMap( const TArray<UMovieSceneTrack*>& Tracks, const TArray<UObject*>& RuntimeObjects, FMovieSceneInstanceMap& TrackInstances, class IMovieScenePlayer& Player );
private:
	/** A paring of a runtime object guid to a runtime data and track instances animating the runtime objects */
	struct FMovieSceneObjectBindingInstance
	{
		/** Runtime object guid for looking up runtime UObjects */
		FGuid ObjectGuid;
		/** Actual runtime objects */	
		TArray<UObject*> RuntimeObjects;
		/** Instances that animate the runtime objects */
		FMovieSceneInstanceMap TrackInstances;
	};

	/** MovieScene that is instanced */
	TWeakObjectPtr<UMovieScene> MovieScene;
	/** The shot track instance map */
	TSharedPtr<class IMovieSceneTrackInstance> ShotTrackInstance;
	/** All Master track instances */
	FMovieSceneInstanceMap MasterTrackInstances;
	/** All object binding instances */
	TMap<FGuid, FMovieSceneObjectBindingInstance> ObjectBindingInstances;
};


