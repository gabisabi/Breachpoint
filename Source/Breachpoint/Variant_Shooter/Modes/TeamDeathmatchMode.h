// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "TeamDeathmatchMode.generated.h"

/**
 *  Team Deathmatch: you against the enemy team, first to the target score wins.
 *  Enemy kills score for you; your deaths score for the enemy.
 */
UCLASS()
class BREACHPOINT_API ATeamDeathmatchMode : public ABreachpointModeBase
{
	GENERATED_BODY()

public:

	ATeamDeathmatchMode();

protected:

	/** Score needed to win the match */
	UPROPERTY(EditAnywhere, Category="Team Deathmatch", meta = (ClampMin = 1, ClampMax = 100))
	int32 TargetScore = 15;

	/** Minimum simultaneous enemies kept in the arena */
	UPROPERTY(EditAnywhere, Category="Team Deathmatch", meta = (ClampMin = 0, ClampMax = 20))
	int32 MinEnemies = 4;

	int32 PlayerScore = 0;
	int32 EnemyScore = 0;

	FTimerHandle TopUpTimer;

protected:

	virtual void BeginPlay() override;

	virtual void OnEnemyKilled(uint8 TeamByte) override;

	virtual void OnPlayerDied() override;

	/** Keeps the enemy population topped up */
	void TopUpEnemies();

	/** Refreshes the objective line */
	void UpdateObjective();
};
