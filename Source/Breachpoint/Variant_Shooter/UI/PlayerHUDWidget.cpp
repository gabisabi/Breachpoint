// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerHUDWidget.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"

void UPlayerHUDWidget::SetupForCharacter(AShooterCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	// Unbind from the previous character if any
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnBulletCountUpdated.RemoveDynamic(this, &UPlayerHUDWidget::HandleBulletCountUpdated);
		BoundCharacter->OnDamaged.RemoveDynamic(this, &UPlayerHUDWidget::HandleDamaged);
	}

	BoundCharacter = Character;

	// Bind to the character's delegates
	Character->OnBulletCountUpdated.AddDynamic(this, &UPlayerHUDWidget::HandleBulletCountUpdated);
	Character->OnDamaged.AddDynamic(this, &UPlayerHUDWidget::HandleDamaged);

	// Push initial state to the Blueprint layer using the character's actual health
	const float HP = Character->GetCurrentHP();
	const float Max = Character->GetMaxHP();
	const float Pct = (Max > 0.0f) ? (HP / Max) : 1.0f;
	BP_UpdateHealth(HP, Max, Pct);

	if (!CachedModeName.IsEmpty())
	{
		BP_UpdateGameModeInfo(CachedModeName, CachedObjective);
	}
}

void UPlayerHUDWidget::SetGameModeInfo(const FString& ModeName, const FString& Objective)
{
	CachedModeName = ModeName;
	CachedObjective = Objective;
	BP_UpdateGameModeInfo(ModeName, Objective);
}

void UPlayerHUDWidget::HandleBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	BP_UpdateAmmo(Bullets, MagazineSize);
}

void UPlayerHUDWidget::HandleDamaged(float LifePercent)
{
	BP_UpdateHealth(LifePercent, 1.0f, LifePercent);
	BP_OnDamageReceived(1.0f - LifePercent, FVector::ZeroVector);
}
