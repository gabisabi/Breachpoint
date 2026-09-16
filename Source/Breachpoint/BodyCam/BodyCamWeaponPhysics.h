// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BodyCamTypes.h"
#include "BodyCamWeaponPhysics.generated.h"

class AShooterWeapon;
class USkeletalMeshComponent;

/**
 *  Realistic weapon physics for the body-cam tactical shooter.
 *
 *  Attach to a weapon actor (or the character holding it) to get:
 *    - Procedural weapon sway (sine + Perlin noise, modulated by movement/stance)
 *    - CS-style recoil spray patterns with recovery
 *    - Two-handed / stance / stamina stability multipliers
 *    - Bullet ballistics data (consumed by projectile or hitscan trace)
 *    - Weight-based handling (ADS speed, movement penalty)
 *
 *  The component does NOT directly move meshes – it exposes offsets that the
 *  owning actor's Tick or AnimBP should read and apply, keeping coupling low.
 */
UCLASS(ClassGroup = (BodyCam), meta = (BlueprintSpawnableComponent))
class BREACHPOINT_API UBodyCamWeaponPhysics : public UActorComponent
{
	GENERATED_BODY()

public:

	UBodyCamWeaponPhysics();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// ── Weapon Identity ─────────────────────────────────────────────────

	/** Weight of the weapon in kg – drives sway, ADS speed, move penalty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Weight",
		meta = (ClampMin = "0.5", ClampMax = "15.0", Units = "kg"))
	float WeaponWeight = 3.5f;

	/** Time to transition into ADS (seconds). Heavier weapons are slower. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Weight",
		meta = (ClampMin = "0.1", ClampMax = "2.0", Units = "s"))
	float BaseADSTime = 0.25f;

	/** Movement speed penalty per kg of weapon weight (fraction). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Weight",
		meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float WeightSpeedPenaltyPerKg = 0.02f;

	/** Whether the weapon currently has a suppressor attached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Attachments")
	bool bHasSuppressor = false;

	/** Muzzle flash intensity multiplier (0 when suppressed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Attachments",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MuzzleFlashScale = 1.0f;

	/** Shot loudness multiplier for AI perception (reduced when suppressed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Attachments",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuppressedLoudnessScale = 0.2f;

	// ── Sway ────────────────────────────────────────────────────────────

	/** Base amplitude of the idle sway (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway",
		meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BaseSwayAmplitude = 0.8f;

	/** Frequency of the primary sway sine wave (Hz). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway",
		meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float SwayFrequency = 1.2f;

	/** Additional sway from movement speed (amplitude added per 100 cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway",
		meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MovementSwayGain = 0.3f;

	/** Noise octave added on top of sine for organic feel (Perlin scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sway",
		meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SwayNoiseScale = 0.4f;

	// ── Recoil ──────────────────────────────────────────────────────────

	/** Spray pattern definition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	FRecoilPattern RecoilPattern;

	/** Current index in the recoil pattern (advances per shot). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Recoil")
	int32 CurrentRecoilIndex = 0;

	/** Accumulated recoil offset (degrees, applied to aim direction). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Recoil")
	FVector2D AccumulatedRecoil = FVector2D::ZeroVector;

	// ── Stability Multipliers ───────────────────────────────────────────

	/** Current stance – set by the movement system. */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Stability")
	EStance CurrentStance = EStance::Standing;

	/** Is the weapon held with both hands? (from VRCharacter two-handed grip) */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Stability")
	bool bTwoHandedGrip = false;

	/** Current stamina fraction (0–1), fed from the movement system. */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Stability",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StaminaFraction = 1.0f;

	/** Stability multipliers per stance (lower = more stable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stability")
	float StandingStabilityMult = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stability")
	float CrouchingStabilityMult = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stability")
	float ProneStabilityMult = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stability")
	float TwoHandedStabilityMult = 0.4f;

	// ── Ballistics ──────────────────────────────────────────────────────

	/** Bullet physics data for this weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ballistics")
	FBulletBallisticsData BallisticsData;

	// ── Blueprint-readable outputs ──────────────────────────────────────

	/** Current frame's procedural sway offset (degrees, X = yaw, Y = pitch). */
	UFUNCTION(BlueprintPure, Category = "Weapon|Sway")
	FVector2D GetSwayOffset() const { return CurrentSwayOffset; }

	/** Effective ADS time accounting for weapon weight. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Weight")
	float GetEffectiveADSTime() const;

	/** Movement speed multiplier from weapon weight. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Weight")
	float GetMovementSpeedMultiplier() const;

	/** Effective loudness (accounting for suppressor). */
	UFUNCTION(BlueprintPure, Category = "Weapon|Attachments")
	float GetEffectiveLoudness() const;

	/** Effective muzzle flash scale (accounting for suppressor). */
	UFUNCTION(BlueprintPure, Category = "Weapon|Attachments")
	float GetEffectiveMuzzleFlashScale() const;

	// ── Recoil API ──────────────────────────────────────────────────────

	/** Call when a shot is fired – advances the recoil pattern. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void ApplyRecoilStep();

	/** Call when the trigger is released – begins recovery to centre. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void BeginRecoilRecovery();

	/** Reset recoil state (e.g. on weapon swap or reload). */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void ResetRecoil();

protected:

	/** Compute the composite stability multiplier from all sources. */
	float ComputeStabilityMultiplier() const;

	/** Procedural sway update. */
	void UpdateSway(float DeltaTime);

	/** Recoil recovery update (per-frame drift back to zero). */
	void UpdateRecoilRecovery(float DeltaTime);

private:

	/** Current sway offset this frame (degrees). */
	FVector2D CurrentSwayOffset = FVector2D::ZeroVector;

	/** Running timer for sway sine wave. */
	float SwayTime = 0.0f;

	/** True while recovery is active (trigger released). */
	bool bRecoveringRecoil = false;

	/** Owner's movement speed last frame (cm/s), used for sway scaling. */
	float OwnerSpeed = 0.0f;
};
