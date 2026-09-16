// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "BodyCamTypes.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────────────────────────────────────

/** Character stance – affects recoil stability, speed, and noise profile. */
UENUM(BlueprintType)
enum class EStance : uint8
{
	Standing   UMETA(DisplayName = "Standing"),
	Crouching  UMETA(DisplayName = "Crouching"),
	Prone      UMETA(DisplayName = "Prone")
};

/** Fire selector for weapons. */
UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Semi     UMETA(DisplayName = "Semi-Auto"),
	Burst    UMETA(DisplayName = "Burst"),
	FullAuto UMETA(DisplayName = "Full Auto")
};

/** Night-vision / camera overlay mode. */
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Normal      UMETA(DisplayName = "Normal"),
	NightVision UMETA(DisplayName = "Night Vision (IR)"),
	Thermal     UMETA(DisplayName = "Thermal")
};

// ─────────────────────────────────────────────────────────────────────────────
//  Structs
// ─────────────────────────────────────────────────────────────────────────────

/**
 *  Ballistics parameters for a single projectile type.
 *  Consumed by BodyCamWeaponPhysics to simulate bullet flight, drop, and penetration.
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FBulletBallisticsData
{
	GENERATED_BODY()

	/** Speed the bullet leaves the barrel (cm/s – UE units). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballistics",
		meta = (ClampMin = "1000.0", Units = "cm/s"))
	float MuzzleVelocity = 90000.0f; // ~900 m/s, typical rifle round

	/** Bullet mass in grams – heavier rounds retain energy better. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballistics",
		meta = (ClampMin = "0.1", Units = "g"))
	float BulletMass = 4.0f; // ~62-grain 5.56

	/** Drag coefficient (unitless) – affects velocity falloff over distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballistics",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DragCoefficient = 0.3f;

	/** Maximum depth of material the bullet can penetrate (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballistics",
		meta = (ClampMin = "0.0", Units = "cm"))
	float MaxPenetrationDepth = 15.0f;

	/** Distance at which damage begins to fall off (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballistics",
		meta = (ClampMin = "0.0", Units = "cm"))
	float DamageDropoffRange = 5000.0f;

	/** Gravity scale applied to the bullet (1.0 = world gravity). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ballistics",
		meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float GravityScale = 1.0f;
};

/**
 *  Defines a weapon's recoil spray pattern (similar to CS-style fixed patterns).
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FRecoilPattern
{
	GENERATED_BODY()

	/**
	 *  Ordered list of 2-D offsets (X = horizontal, Y = vertical/muzzle climb).
	 *  Each entry is consumed per shot. After the last entry the pattern loops
	 *  from a configurable index.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	TArray<FVector2D> Pattern;

	/** Speed at which the recoil recovers back to centre when not firing (degrees/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil",
		meta = (ClampMin = "0.0"))
	float RecoveryRate = 8.0f;

	/** Multiplier applied to the very first shot's recoil offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil",
		meta = (ClampMin = "0.0"))
	float FirstShotMultiplier = 1.5f;

	/** Index in Pattern to loop from once the array is exhausted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil",
		meta = (ClampMin = "0"))
	int32 LoopStartIndex = 5;
};

/**
 *  Per-physical-material surface response (footsteps, speed modifiers).
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FSurfaceResponse
{
	GENERATED_BODY()

	/** Volume multiplier for footstep audio (1.0 = normal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface",
		meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float FootstepVolume = 1.0f;

	/** Movement speed multiplier on this surface (e.g. 0.8 for mud). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface",
		meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SpeedModifier = 1.0f;

	/** Sound cue to play for footsteps on this surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	TSoftObjectPtr<USoundBase> FootstepSound;
};

/**
 *  Settings bundle for the body-cam post-process look.
 *  Exposed so designers can save presets (e.g. "daytime", "night op").
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FBodyCamSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FilmGrainIntensity = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ChromaticAberrationIntensity = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VignetteIntensity = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DesaturationAmount = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowLightNoiseGain = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam")
	bool bShowTimestamp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam")
	bool bShowRecordingIndicator = true;
};
