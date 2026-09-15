// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponSlotInfo.h"
#include "PlayerHUDWidget.generated.h"

class AShooterCharacter;

/**
 *  Centralized HUD widget for the shooter game mode.
 *  Exposes BlueprintImplementableEvents so that a Blueprint subclass
 *  can lay out and animate all HUD elements without C++ coupling.
 *  Call SetupForCharacter to bind to the character's delegates.
 */
UCLASS(abstract)
class BREACHPOINT_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ── Setup ──────────────────────────────────────────────────

	/** Binds this widget to the given character's delegates (ammo, health, damage) */
	UFUNCTION(BlueprintCallable, Category="PlayerHUD")
	void SetupForCharacter(AShooterCharacter* Character);

	/** Sets the current game mode display info */
	UFUNCTION(BlueprintCallable, Category="PlayerHUD")
	void SetGameModeInfo(const FString& ModeName, const FString& Objective);

	// ── Blueprint Implementable Events ────────────────────────

	/** Called when the character's health changes */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="Update Health"))
	void BP_UpdateHealth(float CurrentHP, float MaxHP, float Percentage);

	/** Called when ammo count changes on the current weapon */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="Update Ammo"))
	void BP_UpdateAmmo(int32 CurrentAmmo, int32 MagazineSize);

	/** Called when the weapon inventory changes or the active weapon switches */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="Update Weapon Inventory"))
	void BP_UpdateWeaponInventory(const TArray<FWeaponSlotInfo>& Weapons, int32 ActiveIndex);

	/** Called when a reload begins */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="On Reload Started"))
	void BP_OnReloadStarted(float ReloadDuration);

	/** Called when a reload finishes */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="On Reload Finished"))
	void BP_OnReloadFinished();

	/** Called when the character takes damage */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="On Damage Received"))
	void BP_OnDamageReceived(float DamageAmount, FVector DamageDirection);

	/** Called when the player confirms a kill */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="On Kill Confirmed"))
	void BP_OnKillConfirmed(const FString& VictimName);

	/** Called to update the minimap with the player's current position and rotation */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="Update Minimap"))
	void BP_UpdateMinimap(FVector PlayerLocation, float PlayerYaw);

	/** Called to show a hit marker (optionally a kill variant) */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="Show Hit Marker"))
	void BP_ShowHitMarker(bool bKill);

	/** Called when game mode info is set or updated */
	UFUNCTION(BlueprintImplementableEvent, Category="PlayerHUD", meta=(DisplayName="Update Game Mode Info"))
	void BP_UpdateGameModeInfo(const FString& GameModeName, const FString& ObjectiveText);

protected:

	/** The character this HUD is currently bound to */
	UPROPERTY(BlueprintReadOnly, Category="PlayerHUD")
	TWeakObjectPtr<AShooterCharacter> BoundCharacter;

	/** Cached game mode display name */
	UPROPERTY(BlueprintReadOnly, Category="PlayerHUD")
	FString CachedModeName;

	/** Cached objective text */
	UPROPERTY(BlueprintReadOnly, Category="PlayerHUD")
	FString CachedObjective;

private:

	/** Delegate callback: ammo updated */
	UFUNCTION()
	void HandleBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Delegate callback: character damaged */
	UFUNCTION()
	void HandleDamaged(float LifePercent);
};
