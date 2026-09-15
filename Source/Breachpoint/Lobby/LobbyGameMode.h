// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Lobby/LobbyTypes.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlayerController;

/**
 * Game mode for the lobby level. Manages available maps, game modes,
 * and initiates server travel when a match is started.
 */
UCLASS()
class BREACHPOINT_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	//~ Begin AGameModeBase Interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	//~ End AGameModeBase Interface

	/** All maps available for selection in the lobby */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|Maps")
	TArray<FMapInfo> AvailableMaps;

	/** All game modes available for selection in the lobby */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby|GameModes")
	TArray<FGameModeInfo> AvailableGameModes;

	/** Returns the available maps list */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	const TArray<FMapInfo>& GetAvailableMaps() const { return AvailableMaps; }

	/** Returns the available game modes list */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	const TArray<FGameModeInfo>& GetAvailableGameModes() const { return AvailableGameModes; }

	/**
	 * Initiates server travel to the selected map with the selected game mode.
	 * @param MapPath		Full path to the map asset (e.g. "/Game/Maps/Arena")
	 * @param GameModeClass	The game mode class to load on the target map
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Lobby")
	void StartMatch(const FString& MapPath, TSubclassOf<AGameModeBase> GameModeClass);

protected:
	/** Populates the default maps list. Override in Blueprint or subclass to customize. */
	virtual void PopulateDefaultMaps();

	/** Populates the default game modes list. Override in Blueprint or subclass to customize. */
	virtual void PopulateDefaultGameModes();
};
