// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponSlotInfo.generated.h"

/**
 *  Describes a single weapon slot in the player's inventory for UI display
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FWeaponSlotInfo
{
	GENERATED_BODY()

	/** Display name of the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FString WeaponName;

	/** Current ammo in the magazine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int32 CurrentAmmo = 0;

	/** Maximum magazine capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int32 MagazineSize = 0;

	/** Optional icon texture for the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UTexture2D* WeaponIcon = nullptr;

	/** Whether this slot is the currently active weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	bool bIsActive = false;
};
