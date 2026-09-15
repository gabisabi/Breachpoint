// Copyright Breachpoint. All Rights Reserved.

#include "VR/VRCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputActionValue.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ── VR Camera ──────────────────────────────────
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(GetCapsuleComponent());
	VRCamera->bUsePawnControlRotation = false; // HMD drives rotation directly in VR

	// Disable the inherited first-person camera so HMD only feeds VRCamera
	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetActive(false);
	}

	// ── Motion Controllers ─────────────────────────
	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	LeftMotionController->SetupAttachment(GetRootComponent());
	LeftMotionController->SetTrackingMotionSource(FName("Left"));

	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	RightMotionController->SetupAttachment(GetRootComponent());
	RightMotionController->SetTrackingMotionSource(FName("Right"));

	// ── Hand Meshes ────────────────────────────────
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftMotionController);
	LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightMotionController);
	RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ── Teleport Arc ───────────────────────────────
	TeleportArcSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportArcSpline"));
	TeleportArcSpline->SetupAttachment(LeftMotionController);
	TeleportArcSpline->SetVisibility(false);

	TeleportDestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportDestinationMarker"));
	TeleportDestinationMarker->SetupAttachment(GetRootComponent());
	TeleportDestinationMarker->SetVisibility(false);
	TeleportDestinationMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ── Movement Defaults ──────────────────────────
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NormalWalkSpeed;
		MoveComp->NavAgentProps.bCanCrouch = true;
	}
}

// ────────────────────────────────────────────────────
//  Lifecycle
// ────────────────────────────────────────────────────

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Cache the default ground friction for slide restoration
	if (const UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		DefaultGroundFriction = MoveComp->GroundFriction;
	}

	// Add VR input mapping context
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (VRInputMappingContext)
			{
				Subsystem->AddMappingContext(VRInputMappingContext, 1);
			}
		}
	}

	// Reset HMD orientation at start
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Continuously evaluate two-handed grip while left grip is held
	if (bLeftGripHeld)
	{
		UpdateTwoHandedGrip();
	}

	// Update teleport arc while aiming
	if (bIsTeleporting && bUseTeleportLocomotion)
	{
		UpdateTeleportArc();
	}
}

// ────────────────────────────────────────────────────
//  Input Binding
// ────────────────────────────────────────────────────

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// Grip
	if (IA_GripLeft)
	{
		EIC->BindAction(IA_GripLeft, ETriggerEvent::Started, this, &AVRCharacter::OnGripLeftStarted);
		EIC->BindAction(IA_GripLeft, ETriggerEvent::Completed, this, &AVRCharacter::OnGripLeftCompleted);
	}
	if (IA_GripRight)
	{
		EIC->BindAction(IA_GripRight, ETriggerEvent::Started, this, &AVRCharacter::OnGripRightStarted);
		EIC->BindAction(IA_GripRight, ETriggerEvent::Completed, this, &AVRCharacter::OnGripRightCompleted);
	}

	// Sprint (left thumbstick click)
	if (IA_Sprint)
	{
		EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AVRCharacter::OnSprintStarted);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AVRCharacter::OnSprintCompleted);
	}

	// Crouch (right thumbstick click)
	if (IA_VRCrouch)
	{
		EIC->BindAction(IA_VRCrouch, ETriggerEvent::Started, this, &AVRCharacter::OnCrouchStarted);
		EIC->BindAction(IA_VRCrouch, ETriggerEvent::Completed, this, &AVRCharacter::OnCrouchCompleted);
	}

	// Teleport
	if (IA_Teleport)
	{
		EIC->BindAction(IA_Teleport, ETriggerEvent::Started, this, &AVRCharacter::OnTeleportStarted);
		EIC->BindAction(IA_Teleport, ETriggerEvent::Completed, this, &AVRCharacter::OnTeleportCompleted);
	}
	if (IA_TeleportDirection)
	{
		EIC->BindAction(IA_TeleportDirection, ETriggerEvent::Triggered, this, &AVRCharacter::OnTeleportDirectionUpdated);
	}
}

// ────────────────────────────────────────────────────
//  Weapon Overrides
// ────────────────────────────────────────────────────

void AVRCharacter::AttachWeaponMeshes()
{
	// Attach weapon to right motion controller instead of the first-person mesh
	if (RightMotionController)
	{
		// Subclasses or the weapon system should call AttachToComponent on the
		// weapon mesh actor with this component.  We delegate to the base which
		// normally attaches to FirstPersonMesh, but redirect the attach target.
		// If AShooterCharacter exposes a socket-based attach, override the socket
		// parent here.  Fallback: call Super and re-parent.
		Super::AttachWeaponMeshes();

		// Re-parent the first-person weapon mesh to the right controller
		if (FirstPersonMesh)
		{
			FirstPersonMesh->AttachToComponent(
				RightMotionController,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}
}

FVector AVRCharacter::GetWeaponTargetLocation() const
{
	if (RightMotionController)
	{
		const FVector MuzzleOrigin = RightMotionController->GetComponentLocation();
		const FVector ForwardDir = RightMotionController->GetForwardVector();
		return MuzzleOrigin + ForwardDir * WeaponTraceDistance;
	}

	return Super::GetWeaponTargetLocation();
}

// ────────────────────────────────────────────────────
//  Grip Input
// ────────────────────────────────────────────────────

void AVRCharacter::OnGripLeftStarted(const FInputActionValue& /*Value*/)
{
	bLeftGripHeld = true;
	UpdateTwoHandedGrip();
}

void AVRCharacter::OnGripLeftCompleted(const FInputActionValue& /*Value*/)
{
	bLeftGripHeld = false;

	if (bTwoHandedGripActive)
	{
		bTwoHandedGripActive = false;
		RecoilMultiplier = 1.0f;
	}
}

void AVRCharacter::OnGripRightStarted(const FInputActionValue& /*Value*/)
{
	bRightGripHeld = true;
}

void AVRCharacter::OnGripRightCompleted(const FInputActionValue& /*Value*/)
{
	bRightGripHeld = false;

	if (bTwoHandedGripActive)
	{
		bTwoHandedGripActive = false;
		RecoilMultiplier = 1.0f;
	}
}

// ────────────────────────────────────────────────────
//  Two-Handed Grip
// ────────────────────────────────────────────────────

void AVRCharacter::UpdateTwoHandedGrip()
{
	if (!LeftMotionController || !RightMotionController)
	{
		return;
	}

	const float Distance = FVector::Dist(
		LeftMotionController->GetComponentLocation(),
		RightMotionController->GetComponentLocation());

	const bool bShouldGrip = bLeftGripHeld && bRightGripHeld && (Distance <= TwoHandedGripRadius);

	if (bShouldGrip && !bTwoHandedGripActive)
	{
		bTwoHandedGripActive = true;
		RecoilMultiplier = TwoHandedRecoilMultiplier;

		// Optional haptic feedback on grip engage
		if (GripHapticEffect)
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->PlayHapticEffect(GripHapticEffect, EControllerHand::Left);
			}
		}
	}
	else if (!bShouldGrip && bTwoHandedGripActive)
	{
		bTwoHandedGripActive = false;
		RecoilMultiplier = 1.0f;
	}
}

// ────────────────────────────────────────────────────
//  Sprint
// ────────────────────────────────────────────────────

void AVRCharacter::OnSprintStarted(const FInputActionValue& /*Value*/)
{
	StartSprint();
}

void AVRCharacter::OnSprintCompleted(const FInputActionValue& /*Value*/)
{
	StopSprint();
}

void AVRCharacter::StartSprint()
{
	if (bIsSliding)
	{
		return;
	}

	bIsSprinting = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = SprintSpeed;
	}

	// If already crouching while sprint starts → trigger slide
	if (bIsCrouching)
	{
		StartSlide();
	}
}

void AVRCharacter::StopSprint()
{
	bIsSprinting = false;

	if (!bIsSliding)
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = NormalWalkSpeed;
		}
	}
}

// ────────────────────────────────────────────────────
//  Crouch
// ────────────────────────────────────────────────────

void AVRCharacter::OnCrouchStarted(const FInputActionValue& /*Value*/)
{
	bIsCrouching = true;
	Crouch();

	// Sprint + Crouch → Slide
	if (bIsSprinting)
	{
		StartSlide();
	}
}

void AVRCharacter::OnCrouchCompleted(const FInputActionValue& /*Value*/)
{
	bIsCrouching = false;
	UnCrouch();
}

// ────────────────────────────────────────────────────
//  Slide
// ────────────────────────────────────────────────────

void AVRCharacter::StartSlide()
{
	if (bIsSliding)
	{
		return;
	}

	bIsSliding = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// Reduce friction for the slide
		MoveComp->GroundFriction = SlideFriction;
		MoveComp->BrakingDecelerationWalking = 200.0f;

		// Launch the character forward
		const FVector SlideDirection = GetActorForwardVector();
		LaunchCharacter(SlideDirection * SlideImpulse, true, false);
	}

	// Ensure crouching during slide (lowered camera / capsule)
	if (!bIsCrouching)
	{
		Crouch();
	}

	// Set a timer to end the slide
	GetWorldTimerManager().SetTimer(
		SlideTimerHandle,
		this,
		&AVRCharacter::OnSlideTimerExpired,
		SlideTime,
		false);
}

void AVRCharacter::OnSlideTimerExpired()
{
	EndSlide();
}

void AVRCharacter::EndSlide()
{
	if (!bIsSliding)
	{
		return;
	}

	bIsSliding = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->GroundFriction = DefaultGroundFriction;
		MoveComp->BrakingDecelerationWalking = MoveComp->GetClass()->GetDefaultObject<UCharacterMovementComponent>()->BrakingDecelerationWalking;
		MoveComp->MaxWalkSpeed = bIsSprinting ? SprintSpeed : NormalWalkSpeed;
	}

	if (!bIsCrouching)
	{
		UnCrouch();
	}
}

// ────────────────────────────────────────────────────
//  Teleport
// ────────────────────────────────────────────────────

void AVRCharacter::OnTeleportStarted(const FInputActionValue& /*Value*/)
{
	if (!bUseTeleportLocomotion)
	{
		return;
	}

	bIsTeleporting = true;
	bTeleportDestinationValid = false;

	if (TeleportArcSpline)
	{
		TeleportArcSpline->SetVisibility(true);
	}
}

void AVRCharacter::OnTeleportCompleted(const FInputActionValue& /*Value*/)
{
	if (!bIsTeleporting)
	{
		return;
	}

	bIsTeleporting = false;

	if (TeleportArcSpline)
	{
		TeleportArcSpline->SetVisibility(false);
	}
	if (TeleportDestinationMarker)
	{
		TeleportDestinationMarker->SetVisibility(false);
	}

	if (bTeleportDestinationValid)
	{
		ExecuteTeleport();
	}

	bTeleportDestinationValid = false;
}

void AVRCharacter::OnTeleportDirectionUpdated(const FInputActionValue& Value)
{
	const FVector2D ThumbstickInput = Value.Get<FVector2D>();
	if (!ThumbstickInput.IsNearlyZero())
	{
		TeleportFacingRotation = FRotator(0.0f, FMath::Atan2(ThumbstickInput.Y, ThumbstickInput.X) * (180.0f / UE_PI), 0.0f);
	}
}

void AVRCharacter::UpdateTeleportArc()
{
	if (!LeftMotionController || !TeleportArcSpline)
	{
		return;
	}

	// Predict a projectile path from the left controller
	FPredictProjectilePathParams Params;
	Params.StartLocation = LeftMotionController->GetComponentLocation();
	Params.LaunchVelocity = LeftMotionController->GetForwardVector() * MaxTeleportDistance;
	Params.bTraceWithCollision = true;
	Params.ProjectileRadius = 5.0f;
	Params.MaxSimTime = 2.0f;
	Params.bTraceComplex = false;
	Params.ActorsToIgnore.Add(this);
	Params.SimFrequency = 15.0f;

	FPredictProjectilePathResult Result;
	const bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);

	// Update spline points
	TeleportArcSpline->ClearSplinePoints(false);
	for (const FPredictProjectilePathPointData& Point : Result.PathData)
	{
		TeleportArcSpline->AddSplinePoint(Point.Location, ESplineCoordinateSpace::World, false);
	}
	TeleportArcSpline->UpdateSpline();

	// Validate landing on navmesh
	bTeleportDestinationValid = false;
	if (bHit)
	{
		FNavLocation NavLocation;
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys && NavSys->ProjectPointToNavigation(Result.HitResult.Location, NavLocation, FVector(100.0f)))
		{
			bTeleportDestinationValid = true;
			TeleportDestination = NavLocation.Location;

			if (TeleportDestinationMarker)
			{
				TeleportDestinationMarker->SetWorldLocation(TeleportDestination);
				TeleportDestinationMarker->SetVisibility(true);
			}
		}
	}

	if (!bTeleportDestinationValid && TeleportDestinationMarker)
	{
		TeleportDestinationMarker->SetVisibility(false);
	}
}

void AVRCharacter::ExecuteTeleport()
{
	if (!bTeleportDestinationValid)
	{
		return;
	}

	// Apply facing rotation from thumbstick direction
	SetActorLocationAndRotation(
		TeleportDestination,
		TeleportFacingRotation);

	// Reset the facing for next teleport
	TeleportFacingRotation = GetActorRotation();
}
