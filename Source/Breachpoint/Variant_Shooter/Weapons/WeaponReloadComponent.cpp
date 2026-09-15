// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Weapons/WeaponReloadComponent.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"

UWeaponReloadComponent::UWeaponReloadComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ── Lifecycle ───────────────────────────────────────────────────────────────

void UWeaponReloadComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owning weapon
	OwningWeapon = Cast<AShooterWeapon>(GetOwner());
}

void UWeaponReloadComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Clean up the reload timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}
}

// ── Reload interface ────────────────────────────────────────────────────────

void UWeaponReloadComponent::StartReload()
{
	if (bIsReloading)
	{
		return;
	}

	// Don't reload if the magazine is already full
	if (OwningWeapon.IsValid())
	{
		if (OwningWeapon->GetBulletCount() >= OwningWeapon->GetMagazineSize())
		{
			return;
		}
	}

	bIsReloading = true;
	ReloadStartTime = GetWorld()->GetTimeSeconds();

	// Broadcast the start delegate
	OnReloadStarted.Broadcast();

	// Schedule the finish
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UWeaponReloadComponent::FinishReload, ReloadTime, false);
}

void UWeaponReloadComponent::FinishReload()
{
	if (!bIsReloading)
	{
		return;
	}

	bIsReloading = false;

	// Clear the timer in case FinishReload was called manually before it expired
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	// Refill the magazine
	if (OwningWeapon.IsValid())
	{
		OwningWeapon->RefillMagazine();
	}

	// Broadcast the finished delegate
	OnReloadFinished.Broadcast();
}

void UWeaponReloadComponent::CancelReload()
{
	if (!bIsReloading)
	{
		return;
	}

	bIsReloading = false;

	// Cancel the pending timer
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
}

// ── Progress ────────────────────────────────────────────────────────────────

float UWeaponReloadComponent::GetReloadProgress() const
{
	if (!bIsReloading || ReloadTime <= 0.0f)
	{
		return 0.0f;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - ReloadStartTime;
	return FMath::Clamp(Elapsed / ReloadTime, 0.0f, 1.0f);
}
