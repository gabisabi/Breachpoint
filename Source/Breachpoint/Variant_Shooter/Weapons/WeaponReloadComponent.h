// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponReloadComponent.generated.h"

class AShooterWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadFinished);

/**
 *  Reload component for shooter weapons.
 *  Manages a timed reload cycle with start/finish/cancel support
 *  and broadcasts delegates for UI and animation hooks.
 *  Attach to any AShooterWeapon to replace the instant auto-reload.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BREACHPOINT_API UWeaponReloadComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UWeaponReloadComponent();

	/** Time in seconds to complete a full reload */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reload", meta = (ClampMin = 0, Units = "s"))
	float ReloadTime = 2.0f;

	/** Fired when a reload begins */
	UPROPERTY(BlueprintAssignable, Category="Reload")
	FOnReloadStarted OnReloadStarted;

	/** Fired when a reload completes successfully */
	UPROPERTY(BlueprintAssignable, Category="Reload")
	FOnReloadFinished OnReloadFinished;

	// ── Interface ───────────────────────────────────────────────────────

	/** Begins the reload sequence. No-op if already reloading or magazine is full */
	UFUNCTION(BlueprintCallable, Category="Reload")
	void StartReload();

	/** Completes the reload, refilling the magazine */
	UFUNCTION(BlueprintCallable, Category="Reload")
	void FinishReload();

	/** Cancels an in-progress reload without refilling */
	UFUNCTION(BlueprintCallable, Category="Reload")
	void CancelReload();

	/** Returns true if a reload is currently in progress */
	UFUNCTION(BlueprintPure, Category="Reload")
	bool IsReloading() const { return bIsReloading; }

	/** Returns the normalized reload progress (0 = just started, 1 = done) */
	UFUNCTION(BlueprintPure, Category="Reload")
	float GetReloadProgress() const;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:

	/** True while a reload is in progress */
	UPROPERTY(VisibleInstanceOnly, Category="Reload")
	bool bIsReloading = false;

	/** Timer driving the reload duration */
	FTimerHandle ReloadTimer;

	/** World time when the current reload started (for progress calculation) */
	float ReloadStartTime = 0.0f;

	/** Cached pointer to the owning weapon */
	TWeakObjectPtr<AShooterWeapon> OwningWeapon;
};
