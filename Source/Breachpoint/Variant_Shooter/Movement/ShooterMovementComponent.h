// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterMovementComponent.generated.h"

/**
 *  Extended movement component for shooter characters.
 *  Adds sprint, slide, crouch improvements, and a tackle lunge attack.
 *  All gameplay-relevant states are replicated for multiplayer.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BREACHPOINT_API UShooterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UShooterMovementComponent();

	// ── Sprint ──────────────────────────────────────────────────────────

	/** Maximum speed while sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Sprint", meta = (ClampMin = 0, Units = "cm/s"))
	float SprintSpeed = 800.0f;

	/** Normal walk speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Sprint", meta = (ClampMin = 0, Units = "cm/s"))
	float WalkSpeed = 600.0f;

	// ── Slide ───────────────────────────────────────────────────────────

	/** Initial speed burst when entering a slide */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Slide", meta = (ClampMin = 0, Units = "cm/s"))
	float SlideSpeed = 1200.0f;

	/** How long a slide lasts before ending automatically */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Slide", meta = (ClampMin = 0, Units = "s"))
	float SlideDuration = 0.8f;

	/** Cooldown before the player may slide again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Slide", meta = (ClampMin = 0, Units = "s"))
	float SlideCooldown = 1.5f;

	/** Ground friction override while sliding (lower = more momentum) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Slide", meta = (ClampMin = 0))
	float SlideGroundFriction = 0.5f;

	/** Camera height offset applied during a slide (negative = lower) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Slide", meta = (Units = "cm"))
	float SlideCameraHeightOffset = -40.0f;

	// ── Crouch ──────────────────────────────────────────────────────────

	/** Movement speed while crouching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Crouch", meta = (ClampMin = 0, Units = "cm/s"))
	float CrouchSpeed = 300.0f;

	/** Interpolation speed for the smooth crouch camera transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Crouch", meta = (ClampMin = 0))
	float CrouchInterpSpeed = 12.0f;

	// ── Tackle ──────────────────────────────────────────────────────────

	/** Damage dealt on tackle contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Tackle", meta = (ClampMin = 0))
	float TackleDamage = 50.0f;

	/** Distance the tackle lunge covers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Tackle", meta = (ClampMin = 0, Units = "cm"))
	float TackleDistance = 500.0f;

	/** Cooldown before the player may tackle again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Tackle", meta = (ClampMin = 0, Units = "s"))
	float TackleCooldown = 3.0f;

	/** Speed of the tackle lunge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Tackle", meta = (ClampMin = 0, Units = "cm/s"))
	float TackleLungeSpeed = 2000.0f;

	// ── Public interface ────────────────────────────────────────────────

	/** Request to start sprinting */
	UFUNCTION(BlueprintCallable, Category="Movement|Sprint")
	void StartSprint();

	/** Request to stop sprinting */
	UFUNCTION(BlueprintCallable, Category="Movement|Sprint")
	void StopSprint();

	/** Attempt to begin a slide (requires sprinting and on ground) */
	UFUNCTION(BlueprintCallable, Category="Movement|Slide")
	void StartSlide();

	/** End the current slide early */
	UFUNCTION(BlueprintCallable, Category="Movement|Slide")
	void EndSlide();

	/** Attempt to perform a tackle lunge */
	UFUNCTION(BlueprintCallable, Category="Movement|Tackle")
	void StartTackle();

	/** Returns true if the character is currently sprinting */
	UFUNCTION(BlueprintPure, Category="Movement|Sprint")
	bool IsSprinting() const { return bWantsToSprint && !IsCrouching() && IsMovingForward(); }

	/** Returns true if the character is sliding */
	UFUNCTION(BlueprintPure, Category="Movement|Slide")
	bool IsSliding() const { return bIsSliding; }

	/** Returns true if the character is performing a tackle */
	UFUNCTION(BlueprintPure, Category="Movement|Tackle")
	bool IsTackling() const { return bIsTackling; }

	/** Returns true if the slide cooldown has elapsed */
	UFUNCTION(BlueprintPure, Category="Movement|Slide")
	bool CanSlide() const { return bCanSlide; }

	/** Returns true if the tackle cooldown has elapsed */
	UFUNCTION(BlueprintPure, Category="Movement|Tackle")
	bool CanTackle() const { return bCanTackle; }

protected:

	// ── Overrides ───────────────────────────────────────────────────────

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual float GetMaxSpeed() const override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

private:

	// ── Replicated state ────────────────────────────────────────────────

	/** True while the player is holding the sprint input */
	UPROPERTY(Replicated)
	bool bWantsToSprint = false;

	/** True while the character is in a slide */
	UPROPERTY(Replicated)
	bool bIsSliding = false;

	/** True while the character is performing a tackle */
	UPROPERTY(Replicated)
	bool bIsTackling = false;

	// ── Cooldown tracking ───────────────────────────────────────────────

	bool bCanSlide = true;
	bool bCanTackle = true;

	FTimerHandle SlideTimerHandle;
	FTimerHandle SlideCooldownHandle;
	FTimerHandle TackleTimerHandle;
	FTimerHandle TackleCooldownHandle;

	/** Saved ground friction to restore after slide */
	float DefaultGroundFriction = 8.0f;

	/** Saved crouch half-height for smooth transitions */
	float DefaultCapsuleHalfHeight = 0.0f;

	// ── Helpers ─────────────────────────────────────────────────────────

	/** Returns true if the character has forward input */
	bool IsMovingForward() const;

	/** Timer callback: slide finished naturally */
	void OnSlideFinished();

	/** Timer callback: slide cooldown elapsed */
	void OnSlideCooldownFinished();

	/** Timer callback: tackle finished */
	void OnTackleFinished();

	/** Timer callback: tackle cooldown elapsed */
	void OnTackleCooldownFinished();

	/** Applies tackle damage to overlapping actors */
	void ApplyTackleDamage();

	/** Replication support */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
