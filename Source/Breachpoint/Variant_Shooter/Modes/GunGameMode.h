// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "GunGameMode.generated.h"

class AShooterWeapon;

/**
 *  Gun Game mode: every few kills upgrades the player to the next weapon.
 *  Win by scoring enough kills with the final weapon.
 */
UCLASS()
class BREACHPOINT_API AGunGameMode : public ABreachpointModeBase
{
	GENERATED_BODY()

public:

	AGunGameMode();

protected:

	/** Weapon progression, first entry is the starting weapon */
	UPROPERTY(EditAnywhere, Category="Gun Game")
	TArray<TSubclassOf<AShooterWeapon>> WeaponProgression;

	/** Kills needed to advance to the next weapon */
	UPROPERTY(EditAnywhere, Category="Gun Game", meta = (ClampMin = 1, ClampMax = 20))
	int32 KillsPerTier = 3;

	/** Minimum simultaneous enemies kept in the arena */
	UPROPERTY(EditAnywhere, Category="Gun Game", meta = (ClampMin = 0, ClampMax = 20))
	int32 MinEnemies = 3;

	int32 Kills = 0;
	int32 Tier = 0;

	FTimerHandle LoadoutTimer;

protected:

	virtual void BeginPlay() override;

	virtual void OnEnemyKilled(uint8 TeamByte) override;

	/** Periodically ensures the player holds the right weapon and enough enemies exist */
	void MaintainMatch();

	/** Strips the player's weapons and grants the current tier weapon */
	void GrantTierWeapon();

	/** Refreshes the objective line */
	void UpdateObjective();
};
