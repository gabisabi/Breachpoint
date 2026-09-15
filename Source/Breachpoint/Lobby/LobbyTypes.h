// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyTypes.generated.h"

/**
 * Information about an available map in the lobby.
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FMapInfo
{
	GENERATED_BODY()

	/** Display name for the map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Maps")
	FString MapName;

	/** Soft reference to the map asset (for async loading) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Maps")
	TSoftObjectPtr<UWorld> MapAsset;

	/** Optional thumbnail texture for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Maps")
	TObjectPtr<UTexture2D> Thumbnail = nullptr;

	/** Maximum number of players supported on this map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Maps", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxPlayers = 16;

	FMapInfo() = default;

	FMapInfo(const FString& InMapName, const TSoftObjectPtr<UWorld>& InMapAsset, int32 InMaxPlayers = 16)
		: MapName(InMapName)
		, MapAsset(InMapAsset)
		, MaxPlayers(InMaxPlayers)
	{
	}
};

/**
 * Information about an available game mode in the lobby.
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FGameModeInfo
{
	GENERATED_BODY()

	/** Display name for the game mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|GameModes")
	FString ModeName;

	/** The game mode class to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|GameModes")
	TSubclassOf<AGameModeBase> ModeClass;

	/** Description of the game mode for the UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|GameModes")
	FString Description;

	FGameModeInfo() = default;

	FGameModeInfo(const FString& InModeName, TSubclassOf<AGameModeBase> InModeClass, const FString& InDescription)
		: ModeName(InModeName)
		, ModeClass(InModeClass)
		, Description(InDescription)
	{
	}
};
