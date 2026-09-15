// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "BreachpointModeBase.generated.h"

class AShooterNPC;
class AShooterCharacter;

/**
 *  Base class for Breachpoint game modes.
 *  Wires up the shooter pawn, controller and UI classes, owns the HUD messaging
 *  (objective line, banners, end screen) and translates team score events into
 *  gameplay events for child modes.
 *
 *  Team convention: the player is team 0, enemy NPCs are any other team.
 */
UCLASS(abstract)
class BREACHPOINT_API ABreachpointModeBase : public AShooterGameMode
{
	GENERATED_BODY()

public:

	ABreachpointModeBase();

protected:

	/** NPC class used by modes that spawn their own enemies */
	UPROPERTY(EditAnywhere, Category="Breachpoint|Enemies")
	TSubclassOf<AShooterNPC> EnemyClass;

	/** URL options used to restart this mode, e.g. "?game=/Script/Breachpoint.HordeGameMode" */
	FString ModeOptions;

	/** Text describing the current objective, drawn at the top of the screen */
	FString ObjectiveText;

	/** Large banner text drawn at screen center */
	FString BannerText;

	/** World time until which the banner stays visible */
	float BannerUntil = 0.0f;

	/** True once the match has been decided */
	bool bMatchEnded = false;

	/** Text for the end-of-match screen */
	FString EndText;

	/** Locations where enemies may be spawned (gathered from level NPC spawners) */
	TArray<FTransform> EnemySpawnPoints;

protected:

	virtual void BeginPlay() override;

public:

	/** Routes team score events to gameplay events for child modes */
	virtual void IncrementTeamScore(uint8 TeamByte) override;

	/** Reloads the current level with this mode's options */
	void RestartMatch();

	/** HUD accessors */
	const FString& GetObjectiveText() const { return ObjectiveText; }
	const FString& GetBannerText() const { return BannerText; }
	float GetBannerUntil() const { return BannerUntil; }
	bool IsMatchEnded() const { return bMatchEnded; }
	const FString& GetEndText() const { return EndText; }

protected:

	/** Called when a non-player team character dies */
	virtual void OnEnemyKilled(uint8 TeamByte) {}

	/** Called when the player dies */
	virtual void OnPlayerDied() {}

	/** Shows a large center-screen banner for the given duration */
	void ShowBanner(const FString& Text, float Duration = 3.0f);

	/** Sets the persistent objective line */
	void SetObjective(const FString& Text);

	/** Ends the match and shows the end screen */
	void EndMatch(const FString& Text);

	/** Collects enemy spawn locations from the level's NPC spawners (PlayerStarts as fallback) */
	void GatherEnemySpawnPoints();

	/** Spawns one enemy NPC at a random gathered spawn point with scaled HP. May return null */
	AShooterNPC* SpawnEnemy(float HealthMultiplier = 1.0f);

	/** Returns the player 0 pawn as a shooter character, or null */
	AShooterCharacter* GetPlayerCharacter() const;
};
