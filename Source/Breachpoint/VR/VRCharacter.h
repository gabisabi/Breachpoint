// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "InputActionValue.h"
#include "VRCharacter.generated.h"

class UMotionControllerComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UNiagaraComponent;
class USplineComponent;
class USplineMeshComponent;
class UStaticMeshComponent;
class UHapticFeedbackEffect_Base;
class AShooterWeapon;

/**
 * VR-specific character extending the Shooter character with motion controller support,
 * two-handed weapon stabilization, VR locomotion (smooth + teleport), sprint, crouch, and slide.
 */
UCLASS(Abstract)
class BREACHPOINT_API AVRCharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	AVRCharacter();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	//~ Begin ACharacter Interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End ACharacter Interface

	// ──────────────────────────────────────────────
	//  Components
	// ──────────────────────────────────────────────

	/** Left motion controller (tracking hand position/rotation). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Controllers")
	TObjectPtr<UMotionControllerComponent> LeftMotionController;

	/** Right motion controller (weapon hand). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Controllers")
	TObjectPtr<UMotionControllerComponent> RightMotionController;

	/** Skeletal mesh representing the left hand. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Hands")
	TObjectPtr<USkeletalMeshComponent> LeftHandMesh;

	/** Skeletal mesh representing the right hand. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Hands")
	TObjectPtr<USkeletalMeshComponent> RightHandMesh;

	/** VR camera that follows HMD tracking; overrides the base first-person camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Camera")
	TObjectPtr<UCameraComponent> VRCamera;

	// ──────────────────────────────────────────────
	//  Teleport
	// ──────────────────────────────────────────────

	/** Spline used to preview the teleport arc. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Teleport")
	TObjectPtr<USplineComponent> TeleportArcSpline;

	/** Destination marker shown at the teleport landing point. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Teleport")
	TObjectPtr<UStaticMeshComponent> TeleportDestinationMarker;

	// ──────────────────────────────────────────────
	//  Input Actions
	// ──────────────────────────────────────────────

	/** VR-specific input mapping context (motion controllers). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputMappingContext> VRInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> IA_GripLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> IA_GripRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> IA_VRCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> IA_Teleport;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputAction> IA_TeleportDirection;

	// ──────────────────────────────────────────────
	//  Locomotion Settings
	// ──────────────────────────────────────────────

	/** Maximum walk speed when sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	float SprintSpeed = 800.0f;

	/** Normal walk speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	float NormalWalkSpeed = 600.0f;

	/** Duration of slide in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	float SlideTime = 0.8f;

	/** Forward impulse applied when initiating a slide. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	float SlideImpulse = 1200.0f;

	/** Ground friction applied while sliding. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	float SlideFriction = 0.1f;

	/** Whether teleport locomotion is enabled (vs smooth locomotion). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	bool bUseTeleportLocomotion = false;

	/** Maximum teleport distance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Locomotion")
	float MaxTeleportDistance = 1500.0f;

	// ──────────────────────────────────────────────
	//  Two-Handed Grip / Recoil
	// ──────────────────────────────────────────────

	/** Distance threshold for left hand to engage two-handed grip on weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Grip")
	float TwoHandedGripRadius = 25.0f;

	/** Recoil multiplier when two-handed grip is active (0 = no recoil). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VR|Grip")
	float TwoHandedRecoilMultiplier = 0.3f;

	/** Current effective recoil multiplier (1.0 = full, reduced when two-handing). */
	UPROPERTY(BlueprintReadOnly, Category = "VR|Grip")
	float RecoilMultiplier = 1.0f;

	// ──────────────────────────────────────────────
	//  Haptics
	// ──────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Haptics")
	TObjectPtr<UHapticFeedbackEffect_Base> GripHapticEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Haptics")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHapticEffect;

	// ──────────────────────────────────────────────
	//  State Queries
	// ──────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "VR|Locomotion")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "VR|Locomotion")
	bool IsSliding() const { return bIsSliding; }

	UFUNCTION(BlueprintPure, Category = "VR|Grip")
	bool IsTwoHandedGripActive() const { return bTwoHandedGripActive; }

	UFUNCTION(BlueprintPure, Category = "VR|Locomotion")
	bool IsVRCrouching() const { return bIsCrouching; }

protected:
	// ──────────────────────────────────────────────
	//  Overrides from AShooterCharacter
	// ──────────────────────────────────────────────

	/** Attach weapon meshes to the right motion controller instead of the FP mesh. */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Return aim target based on right motion controller forward vector. */
	virtual FVector GetWeaponTargetLocation() override;

	// ──────────────────────────────────────────────
	//  Input Handlers
	// ──────────────────────────────────────────────

	void OnGripLeftStarted(const FInputActionValue& Value);
	void OnGripLeftCompleted(const FInputActionValue& Value);
	void OnGripRightStarted(const FInputActionValue& Value);
	void OnGripRightCompleted(const FInputActionValue& Value);

	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);

	void OnCrouchStarted(const FInputActionValue& Value);
	void OnCrouchCompleted(const FInputActionValue& Value);

	void OnTeleportStarted(const FInputActionValue& Value);
	void OnTeleportCompleted(const FInputActionValue& Value);
	void OnTeleportDirectionUpdated(const FInputActionValue& Value);

	// ──────────────────────────────────────────────
	//  Locomotion Internals
	// ──────────────────────────────────────────────

	void StartSprint();
	void StopSprint();
	void StartSlide();
	void EndSlide();
	void UpdateTeleportArc();
	void ExecuteTeleport();

	/** Called via timer when the slide duration elapses. */
	UFUNCTION()
	void OnSlideTimerExpired();

	/** Evaluate whether the left hand is close enough to the weapon for a two-handed grip. */
	void UpdateTwoHandedGrip();

	// ──────────────────────────────────────────────
	//  State
	// ──────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "VR|Locomotion")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR|Locomotion")
	bool bIsCrouching = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR|Locomotion")
	bool bIsSliding = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR|Grip")
	bool bLeftGripHeld = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR|Grip")
	bool bRightGripHeld = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR|Grip")
	bool bTwoHandedGripActive = false;

	/** Whether the player is currently previewing a teleport arc. */
	bool bIsTeleporting = false;

	/** Stored teleport direction from thumbstick while aiming. */
	FRotator TeleportFacingRotation = FRotator::ZeroRotator;

	/** Cached teleport destination (valid only when arc hits navmesh). */
	FVector TeleportDestination = FVector::ZeroVector;
	bool bTeleportDestinationValid = false;

	/** Stored default ground friction, restored after slide ends. */
	float DefaultGroundFriction = 8.0f;

	/** Timer handle for slide duration. */
	FTimerHandle SlideTimerHandle;

	/** Weapon trace distance for target location calculation. */
	UPROPERTY(EditDefaultsOnly, Category = "VR|Weapon")
	float WeaponTraceDistance = 10000.0f;
};
