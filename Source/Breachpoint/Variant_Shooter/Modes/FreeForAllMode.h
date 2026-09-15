// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "FreeForAllMode.generated.h"

class AShooterCharacter;

/**
 *  Free-For-All: every player fights for themselves.
 *  Each player (and bot) is assigned a unique team byte so all
 *  damage is hostile. First to TargetScore kills wins.
 */
UCLASS()
class BREACHPOINT_API AFreeForAllMode : public ABreachpointModeBase
{
	GENERATED_BODY()

public:

	AFreeForAllMode();

protected:

	/** Number of kills required to win the match */
	UPROPERTY(EditAnywhere, Category="Free For All", meta = (ClampMin = 1, ClampMax = 100))
	int32 TargetScore = 20;

	/** Minimum number of bot enemies kept alive in the arena */
	UPROPERTY(EditAnywhere, Category="Free For All", meta = (ClampMin = 0, ClampMax = 20))
	int32 MinEnemies = 5;

	/** Kill counts indexed by team byte (each player/bot gets a unique team byte) */
	UPROPERTY()
	TMap<uint8, int32> KillsByTeam;

	/** Display names per team byte for the leaderboard */
	UPROPERTY()
	TMap<uint8, FString> TeamNames;

	/** Next team byte to assign (0 is the player, 1+ are bots) */
	uint8 NextTeamByte = 1;

	/** Keeps track of the player's team byte */
	uint8 PlayerTeamByte = 0;

	/** Timer for topping up enemy population */
	FTimerHandle TopUpTimer;

protected:

	virtual void BeginPlay() override;

	/** Handles a kill scored by a team */
	virtual void OnEnemyKilled(uint8 TeamByte) override;

	/** Handles the player dying — counts as a kill for a random enemy team */
	virtual void OnPlayerDied() override;

	/** Spawns an enemy with its own unique team byte */
	void SpawnFFAEnemy();

	/** Ensures the enemy population stays topped up */
	void TopUpEnemies();

	/** Refreshes the objective and leaderboard text */
	void UpdateObjective();

	/** Builds a leaderboard string sorted by kills descending */
	FString BuildLeaderboard() const;

public:

	/** Assigns a unique team byte to a newly joining character */
	UFUNCTION(BlueprintCallable, Category="Free For All")
	uint8 AssignTeam(AShooterCharacter* Character);
};
