// Copyright Breachpoint. All Rights Reserved.

#include "TacticalMovement.h"
#include "BodyCamWeaponPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

// Try to include the shooter movement component so we can disable arcade moves
#include "Variant_Shooter/Movement/ShooterMovementComponent.h"

UTacticalMovement::UTacticalMovement()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics; // before CMC processes movement
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::BeginPlay()
{
	Super::BeginPlay();

	CachedCharacter = Cast<ACharacter>(GetOwner());
	if (CachedCharacter)
	{
		CachedCMC = CachedCharacter->GetCharacterMovement();
		if (CachedCMC)
		{
			BaseWalkSpeed = CachedCMC->MaxWalkSpeed;
		}
	}

	// Cache weapon physics if it's on the same actor
	CachedWeaponPhysics = GetOwner()
		? GetOwner()->FindComponentByClass<UBodyCamWeaponPhysics>()
		: nullptr;

	CurrentStamina = MaxStamina;

	DisableArcadeMovement();
}

void UTacticalMovement::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedCharacter || !CachedCMC) return;

	// Re-cache weapon physics if it changes (weapon swap)
	if (!CachedWeaponPhysics)
	{
		CachedWeaponPhysics = GetOwner()->FindComponentByClass<UBodyCamWeaponPhysics>();
	}

	UpdateInertia(DeltaTime);
	UpdateLean(DeltaTime);
	UpdateStamina(DeltaTime);
	UpdateSurfaceDetection();
	ApplySpeedModifiers();

	// Feed stamina to weapon physics for stability calculation
	if (CachedWeaponPhysics)
	{
		CachedWeaponPhysics->StaminaFraction = GetStaminaFraction();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Disable arcade movement (no slide, no tackle)
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::DisableArcadeMovement()
{
	if (!CachedCharacter) return;

	// If the character has a ShooterMovementComponent, zero out slide and tackle
	if (UShooterMovementComponent* ShooterMC =
		Cast<UShooterMovementComponent>(CachedCMC))
	{
		ShooterMC->SlideSpeed = 0.0f;
		ShooterMC->SlideDuration = 0.0f;
		ShooterMC->TackleDamage = 0.0f;
		ShooterMC->TackleDistance = 0.0f;
		ShooterMC->TackleLungeSpeed = 0.0f;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Inertia – smooth acceleration and direction changes
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::UpdateInertia(float DeltaTime)
{
	if (!CachedCMC) return;

	// Read the character's pending input vector (normalised desired direction)
	const FVector DesiredDir = CachedCMC->GetLastInputVector();

	// Smoothly blend toward the desired direction
	const float InterpSpeed = DesiredDir.IsNearlyZero()
		? (1.0f / FMath::Max(DecelerationTime, 0.01f))
		: (1.0f / FMath::Max(AccelerationTime, 0.01f));

	// Inertia factor scales how aggressive the smoothing is
	const float EffectiveInterp = FMath::Lerp(100.0f, InterpSpeed, InertiaFactor);

	SmoothedInputDirection = FMath::VInterpTo(
		SmoothedInputDirection, DesiredDir, DeltaTime, EffectiveInterp);

	// Override the CMC's acceleration to follow our smoothed direction
	// We do this by adjusting BrakingDecelerationWalking dynamically
	if (InertiaFactor > 0.01f)
	{
		const float InertiaDecel = FMath::Lerp(2048.0f, 400.0f, InertiaFactor);
		CachedCMC->BrakingDecelerationWalking = InertiaDecel;

		const float InertiaAccel = FMath::Lerp(2048.0f, 600.0f, InertiaFactor);
		CachedCMC->MaxAcceleration = InertiaAccel;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lean
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::StartLeanLeft()  { bIsLeaningLeft = true;  bIsLeaningRight = false; }
void UTacticalMovement::StopLeanLeft()   { bIsLeaningLeft = false; }
void UTacticalMovement::StartLeanRight() { bIsLeaningRight = true; bIsLeaningLeft = false; }
void UTacticalMovement::StopLeanRight()  { bIsLeaningRight = false; }

void UTacticalMovement::UpdateLean(float DeltaTime)
{
	float TargetAngle = 0.0f;
	if (bIsLeaningLeft)       TargetAngle = -MaxLeanAngle;
	else if (bIsLeaningRight) TargetAngle =  MaxLeanAngle;

	CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, TargetAngle, DeltaTime,
		LeanSpeed / FMath::Max(MaxLeanAngle, 1.0f) * 5.0f);

	// Apply camera roll to the controller for lean visual
	if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
	{
		FRotator ControlRot = PC->GetControlRotation();
		// Only touch Roll – leave Pitch/Yaw to the player
		ControlRot.Roll = CurrentLeanAngle;
		PC->SetControlRotation(ControlRot);
	}

	// TODO: Apply lateral camera translation (LeanTranslation) for peeking.
	//       In VR this needs special handling since the HMD controls the camera.
	//       For flat-screen: offset the camera component by
	//       (CurrentLeanAngle / MaxLeanAngle) * LeanTranslation along the right vector.
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stamina
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::UpdateStamina(float DeltaTime)
{
	const bool bIsSprinting = CachedCMC
		? (CachedCMC->Velocity.Size2D() > BaseWalkSpeed * 0.9f &&
		   CachedCMC->MaxWalkSpeed > BaseWalkSpeed * 1.1f)
		: false;

	if (bIsSprinting)
	{
		// Drain stamina
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * DeltaTime);
		StaminaRecoveryTimer = StaminaRecoveryDelay;

		// Force stop sprinting if out of stamina
		if (CurrentStamina <= 0.0f && CachedCMC)
		{
			CachedCMC->MaxWalkSpeed = BaseWalkSpeed;
		}

		bWasSprintingLastFrame = true;
	}
	else
	{
		// Recovery delay countdown
		if (StaminaRecoveryTimer > 0.0f)
		{
			StaminaRecoveryTimer -= DeltaTime;
		}
		else
		{
			// Recover stamina
			CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRecoveryRate * DeltaTime);
		}

		bWasSprintingLastFrame = false;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Surface Detection
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::UpdateSurfaceDetection()
{
	if (!CachedCharacter) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Trace straight down from the character's feet
	const FVector Start = CachedCharacter->GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, 150.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CachedCharacter);
	Params.bReturnPhysicalMaterial = true;

	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (Hit.PhysMaterial.IsValid())
		{
			CurrentSurface = Hit.PhysMaterial->SurfaceType;
		}
		else
		{
			CurrentSurface = SurfaceType_Default;
		}
	}
}

FSurfaceResponse UTacticalMovement::GetCurrentSurfaceResponse() const
{
	if (const FSurfaceResponse* Found = SurfaceResponses.Find(CurrentSurface))
	{
		return *Found;
	}

	// Default response
	return FSurfaceResponse();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Speed Modifiers
// ─────────────────────────────────────────────────────────────────────────────

void UTacticalMovement::ApplySpeedModifiers()
{
	if (!CachedCMC) return;

	float SpeedMult = 1.0f;

	// Weapon weight penalty
	if (CachedWeaponPhysics)
	{
		SpeedMult *= CachedWeaponPhysics->GetMovementSpeedMultiplier();
	}

	// Stamina depletion penalty
	const float StamFrac = GetStaminaFraction();
	if (StamFrac < 0.2f)
	{
		// Lerp from DepletedSpeedMultiplier at 0% to 1.0 at 20%
		const float T = StamFrac / 0.2f;
		SpeedMult *= FMath::Lerp(DepletedSpeedMultiplier, 1.0f, T);
	}

	// Surface modifier
	const FSurfaceResponse& Surface = GetCurrentSurfaceResponse();
	SpeedMult *= Surface.SpeedModifier;

	// Apply to the CMC
	// NOTE: We write MaxWalkSpeed directly. If sprint is active, the caller
	// (ShooterMovementComponent::GetMaxSpeed) should still layer sprint on top.
	CachedCMC->MaxWalkSpeed = BaseWalkSpeed * SpeedMult;
}
