// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyTypes.h"
#include "LobbyUI.generated.h"

class ALobbyGameMode;
class ALobbyPlayerController;

/**
 * Base widget for the lobby screen. Provides the C++ interface;
 * visual layout and binding are done in a Blueprint subclass (WBP_LobbyUI).
 */
UCLASS(Abstract)
class BREACHPOINT_API ULobbyUI : public UUserWidget
{
	GENERATED_BODY()

public:
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	//~ End UUserWidget Interface

	// ---------------------------------------------------------------
	// Blueprint Implementable Events (implement visuals in Blueprint)
	// ---------------------------------------------------------------

	/** Called to populate the map selection list in the UI */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|UI")
	void BP_PopulateMapList(const TArray<FMapInfo>& Maps);

	/** Called to populate the game mode selection list in the UI */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|UI")
	void BP_PopulateGameModeList(const TArray<FGameModeInfo>& GameModes);

	/** Called when the local player's ready state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|UI")
	void BP_OnPlayerReadyChanged(bool bReady);

	/** Called when the match is about to start (countdown, transition, etc.) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|UI")
	void BP_OnMatchStarting();

	// ---------------------------------------------------------------
	// Blueprint Callable functions (called from Blueprint buttons/events)
	// ---------------------------------------------------------------

	/** Select a map by index into the AvailableMaps array */
	UFUNCTION(BlueprintCallable, Category = "Lobby|UI")
	void SelectMap(int32 MapIndex);

	/** Select a game mode by index into the AvailableGameModes array */
	UFUNCTION(BlueprintCallable, Category = "Lobby|UI")
	void SelectGameMode(int32 ModeIndex);

	/** Toggle the local player's ready state */
	UFUNCTION(BlueprintCallable, Category = "Lobby|UI")
	void ToggleReady();

	/** Request the host to start the match with current selections */
	UFUNCTION(BlueprintCallable, Category = "Lobby|UI")
	void RequestStartMatch();

	/** Returns the currently selected map index */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby|UI")
	int32 GetSelectedMapIndex() const { return SelectedMapIndex; }

	/** Returns the currently selected game mode index */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby|UI")
	int32 GetSelectedGameModeIndex() const { return SelectedGameModeIndex; }

protected:
	/** Currently selected map index (-1 = none) */
	UPROPERTY(BlueprintReadOnly, Category = "Lobby|UI")
	int32 SelectedMapIndex = 0;

	/** Currently selected game mode index (-1 = none) */
	UPROPERTY(BlueprintReadOnly, Category = "Lobby|UI")
	int32 SelectedGameModeIndex = 0;

private:
	/** Convenience: get the owning lobby player controller */
	ALobbyPlayerController* GetLobbyPC() const;

	/** Convenience: get the lobby game mode (only valid on authority / listen server) */
	ALobbyGameMode* GetLobbyGameMode() const;
};
