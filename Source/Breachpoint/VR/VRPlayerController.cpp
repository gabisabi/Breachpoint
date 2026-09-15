// Copyright Breachpoint. All Rights Reserved.

#include "VR/VRPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "VR/VRCharacter.h"

AVRPlayerController::AVRPlayerController()
{
	// VR controllers should not show a mouse cursor
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableTouchEvents = false;
}

// ────────────────────────────────────────────────────
//  Lifecycle
// ────────────────────────────────────────────────────

void AVRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Detect VR mode from HMD availability
	bIsInVRMode = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (bIsInVRMode)
	{
		SetupVRInputMappingContexts();
		DisableMobileControls();
		ResetHMDTracking();
	}
}

void AVRPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (bIsInVRMode)
	{
		HandleRespawn();
	}
}

void AVRPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Additional VR-specific bindings can be configured here if needed.
	// Character-level bindings live in AVRCharacter::SetupPlayerInputComponent.
}

// ────────────────────────────────────────────────────
//  VR Input Mapping
// ────────────────────────────────────────────────────

void AVRPlayerController::SetupVRInputMappingContexts()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (VRBaseMappingContext)
		{
			Subsystem->AddMappingContext(VRBaseMappingContext, VRBaseMappingPriority);
		}

		if (VRLocomotionMappingContext)
		{
			Subsystem->AddMappingContext(VRLocomotionMappingContext, VRLocomotionMappingPriority);
		}
	}
}

// ────────────────────────────────────────────────────
//  HMD / Respawn
// ────────────────────────────────────────────────────

void AVRPlayerController::ResetHMDTracking()
{
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
	}
}

void AVRPlayerController::HandleRespawn()
{
	// Reset HMD so the player faces forward at the new spawn
	ResetHMDTracking();

	// Re-apply VR input contexts in case they were cleared during respawn
	SetupVRInputMappingContexts();
}

// ────────────────────────────────────────────────────
//  Mobile Controls Suppression
// ────────────────────────────────────────────────────

void AVRPlayerController::DisableMobileControls()
{
	// In VR mode we never need on-screen touch controls.
	// The base ShooterPlayerController may create a virtual joystick
	// or mobile HUD; deactivate it.
	bEnableTouchEvents = false;

	// If a touch interface is configured, clear it
	if (CurrentTouchInterface)
	{
		ActivateTouchInterface(nullptr);
	}
}
