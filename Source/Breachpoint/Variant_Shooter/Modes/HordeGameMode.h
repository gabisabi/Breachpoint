// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "HordeGameMode.generated.h"

/**
 *  Horde mode: survive endless waves of enemies.
 *  Each wave spawns more, tougher NPCs. The match ends when the player dies.
 */
UCLASS()
class BREACHPOINT_API AHordeGameMode : public ABreachpointModeBase
{
	GENERATED_BODY()

public:

	AHordeGameMode();

protected:

	/** Enemies in wave 1 */
	UPROPERTY(EditAnywhere, Category="Horde", meta = (ClampMin = 1, ClampMax = 20))
	int32 BaseEnemiesPerWave = 4;

	/** Additional enemies added each wave */
	UPROPERTY(EditAnywhere, Category="Horde", meta = (ClampMin = 0, ClampMax = 10))
	int32 EnemiesAddedPerWave = 2;

	/** Hard cap on simultaneous wave size */
	UPROPERTY(EditAnywhere, Category="Horde", meta = (ClampMin = 1, ClampMax = 50))
	int32 MaxEnemiesPerWave = 22;

	/** Extra enemy HP fraction added per wave */
	UPROPERTY(EditAnywhere, Category="Horde", meta = (ClampMin = 0, ClampMax = 1))
	float HealthGainPerWave = 0.12f;

	/** Seconds between waves */
	UPROPERTY(EditAnywhere, Category="Horde", meta = (ClampMin = 0, ClampMax = 60))
	float IntermissionTime = 7.0f;

	/** Seconds between individual enemy spawns within a wave */
	UPROPERTY(EditAnywhere, Category="Horde", meta = (ClampMin = 0.1, ClampMax = 10))
	float SpawnInterval = 0.7f;

	int32 Wave = 0;
	int32 PendingSpawns = 0;
	int32 AliveEnemies = 0;
	int32 TotalKills = 0;

	FTimerHandle WaveTimer;
	FTimerHandle SpawnTimer;

protected:

	virtual void BeginPlay() override;

	/** Horde manages its own spawning; keep the level's spawners idle */
	virtual bool ShouldSpawnEnemyNPCs() const override { return false; }

	virtual void OnPlayerDied() override;

	/** Starts the next wave */
	void StartNextWave();

	/** Spawns one wave enemy */
	void SpawnWaveEnemy();

	/** Bound to each spawned enemy's death */
	UFUNCTION()
	void OnHordeEnemyDied();

	/** Refreshes the objective line */
	void UpdateObjective();
};
