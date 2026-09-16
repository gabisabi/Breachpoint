// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BodyCamTypes.h"
#include "TacticalMovement.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UBodyCamWeaponPhysics;

/**
 *  Tactical movement overlay for the body-cam shooter.
 *
 *  Attach to any ACharacter to get:
 *    - Movement inertia (smooth accel/decel, no instant direction flips)
 *    - Weapon-weight speed penalty (reads from UBodyCamWeaponPhysics)
 *    - Lean left / right (camera roll + slight translation)
 *    - Stamina system (drain while sprinting, affects aim stability + speed)
 *    - Surface-type footstep detection (physical material trace)
 *    - No slide, no tackle — removed from the arcade movement set
 *
 *  This component does NOT subclass UCharacterMovementComponent. It modifies
 *  the existing CMC's parameters each frame, so it composes with any CMC
 *  subclass (including UShooterMovementComponent).
 */
UCLASS(ClassGroup = (BodyCam), meta = (BlueprintSpawnableComponent))
class BREACHPOINT_API UTacticalMovement : public UActorComponent
{
	GENERATED_BODY()

public:

	UTacticalMovement();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// ── Inertia ─────────────────────────────────────────────────────────

	/**
	 *  0 = instant direction changes (arcade), 1 = very heavy inertia.
	 *  0.3–0.5 feels good for a tactical shooter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Inertia",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InertiaFactor = 0.4f;

	/** Time to reach full speed from standstill (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Inertia",
		meta = (ClampMin = "0.05", ClampMax = "2.0", Units = "s"))
	float AccelerationTime = 0.35f;

	/** Time to decelerate to zero from full speed (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Inertia",
		meta = (ClampMin = "0.05", ClampMax = "2.0", Units = "s"))
	float DecelerationTime = 0.25f;

	// ── Leaning ─────────────────────────────────────────────────────────

	/** Maximum lean angle in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Lean",
		meta = (ClampMin = "0.0", ClampMax = "30.0", Units = "deg"))
	float MaxLeanAngle = 15.0f;

	/** How fast the lean interpolates (degrees per second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Lean",
		meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float LeanSpeed = 90.0f;

	/** Lateral offset when fully leaned (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Lean",
		meta = (ClampMin = "0.0", ClampMax = "50.0", Units = "cm"))
	float LeanTranslation = 25.0f;

	/** Current lean state – set via input or VR head tilt. */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical|Lean")
	bool bIsLeaningLeft = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tactical|Lean")
	bool bIsLeaningRight = false;

	// ── Stamina ─────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Stamina",
		meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Tactical|Stamina")
	float CurrentStamina = 100.0f;

	/** Stamina drained per second while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Stamina",
		meta = (ClampMin = "0.0"))
	float StaminaDrainRate = 15.0f;

	/** Stamina recovered per second while not sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Stamina",
		meta = (ClampMin = "0.0"))
	float StaminaRecoveryRate = 8.0f;

	/** Delay after sprinting before stamina begins recovering (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Stamina",
		meta = (ClampMin = "0.0", Units = "s"))
	float StaminaRecoveryDelay = 1.5f;

	/** Movement speed multiplier when stamina is fully depleted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Stamina",
		meta = (ClampMin = "0.3", ClampMax = "1.0"))
	float DepletedSpeedMultiplier = 0.7f;

	// ── Surface Detection ───────────────────────────────────────────────

	/** Mapping from physical surface type to footstep response. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Surface")
	TMap<TEnumAsByte<EPhysicalSurface>, FSurfaceResponse> SurfaceResponses;

	/** Current surface the character is standing on. */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical|Surface")
	TEnumAsByte<EPhysicalSurface> CurrentSurface;

	// ── Base speed reference ────────────────────────────────────────────

	/** Unmodified walk speed (read from CMC on BeginPlay). */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical|Movement")
	float BaseWalkSpeed = 600.0f;

	/** Unmodified sprint speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical|Movement",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float BaseSprintSpeed = 750.0f;

	// ── Blueprint API ───────────────────────────────────────────────────

	/** Request lean left (call on press; clear on release). */
	UFUNCTION(BlueprintCallable, Category = "Tactical|Lean")
	void StartLeanLeft();

	UFUNCTION(BlueprintCallable, Category = "Tactical|Lean")
	void StopLeanLeft();

	UFUNCTION(BlueprintCallable, Category = "Tactical|Lean")
	void StartLeanRight();

	UFUNCTION(BlueprintCallable, Category = "Tactical|Lean")
	void StopLeanRight();

	/** Stamina fraction (0–1) for UI and weapon stability queries. */
	UFUNCTION(BlueprintPure, Category = "Tactical|Stamina")
	float GetStaminaFraction() const { return CurrentStamina / FMath::Max(MaxStamina, 1.0f); }

	/** Whether the player can currently sprint. */
	UFUNCTION(BlueprintPure, Category = "Tactical|Stamina")
	bool CanSprint() const { return CurrentStamina > 5.0f; }

	/** Get the surface response for the current ground surface. */
	UFUNCTION(BlueprintPure, Category = "Tactical|Surface")
	FSurfaceResponse GetCurrentSurfaceResponse() const;

	/** Current lean angle (negative = left, positive = right). */
	UFUNCTION(BlueprintPure, Category = "Tactical|Lean")
	float GetCurrentLeanAngle() const { return CurrentLeanAngle; }

protected:

	// ── Internals ───────────────────────────────────────────────────────

	void UpdateInertia(float DeltaTime);
	void UpdateLean(float DeltaTime);
	void UpdateStamina(float DeltaTime);
	void UpdateSurfaceDetection();
	void ApplySpeedModifiers();

	/** Disables slide and tackle on the ShooterMovementComponent, if present. */
	void DisableArcadeMovement();

private:

	UPROPERTY()
	TObjectPtr<ACharacter> CachedCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CachedCMC;

	UPROPERTY()
	TObjectPtr<UBodyCamWeaponPhysics> CachedWeaponPhysics;

	/** Current interpolated lean angle. */
	float CurrentLeanAngle = 0.0f;

	/** Smoothed velocity direction for inertia. */
	FVector SmoothedInputDirection = FVector::ZeroVector;

	/** Timer tracking stamina recovery delay. */
	float StaminaRecoveryTimer = 0.0f;

	/** Was the player sprinting last frame? */
	bool bWasSprintingLastFrame = false;
};
