// Copyright Breachpoint. All Rights Reserved.

#include "BodyCamWeaponPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBodyCamWeaponPhysics::UBodyCamWeaponPhysics()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBodyCamWeaponPhysics::BeginPlay()
{
	Super::BeginPlay();
	ResetRecoil();
}

void UBodyCamWeaponPhysics::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cache owner speed for sway calculations
	if (AActor* Owner = GetOwner())
	{
		OwnerSpeed = Owner->GetVelocity().Size();
	}

	UpdateSway(DeltaTime);
	UpdateRecoilRecovery(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Sway
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamWeaponPhysics::UpdateSway(float DeltaTime)
{
	SwayTime += DeltaTime;

	const float Stability = ComputeStabilityMultiplier();

	// Base sine-driven sway
	const float Amplitude = BaseSwayAmplitude * Stability;
	const float SinX = FMath::Sin(SwayTime * SwayFrequency * 2.0f * PI) * Amplitude;
	const float SinY = FMath::Sin(SwayTime * SwayFrequency * 1.37f * 2.0f * PI) * Amplitude * 0.7f;

	// Perlin-like noise via two offset sine waves (cheap approximation)
	const float NoiseX = FMath::Sin(SwayTime * 3.7f + 1.23f) * SwayNoiseScale * Stability * 0.5f;
	const float NoiseY = FMath::Sin(SwayTime * 2.9f + 4.56f) * SwayNoiseScale * Stability * 0.5f;

	// Movement adds to sway proportionally
	const float MoveSway = (OwnerSpeed / 100.0f) * MovementSwayGain * Stability;

	CurrentSwayOffset.X = SinX + NoiseX + FMath::Sin(SwayTime * 5.1f) * MoveSway;
	CurrentSwayOffset.Y = SinY + NoiseY + FMath::Sin(SwayTime * 4.3f) * MoveSway * 0.6f;

	// Weight makes sway feel sluggish – heavier weapons have slower, wider arcs
	// (amplitude already higher via base, but we also slightly slow the frequency)
	// This is baked into BaseSwayAmplitude being set per-weapon; no runtime mod needed.
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stability
// ─────────────────────────────────────────────────────────────────────────────

float UBodyCamWeaponPhysics::ComputeStabilityMultiplier() const
{
	float Mult = 1.0f;

	// Stance
	switch (CurrentStance)
	{
	case EStance::Standing:  Mult *= StandingStabilityMult;  break;
	case EStance::Crouching: Mult *= CrouchingStabilityMult; break;
	case EStance::Prone:     Mult *= ProneStabilityMult;     break;
	}

	// Two-handed grip
	if (bTwoHandedGrip)
	{
		Mult *= TwoHandedStabilityMult;
	}

	// Low stamina destabilises aim (lerp from 1x at full stamina to 1.8x at zero)
	const float StaminaPenalty = FMath::Lerp(1.8f, 1.0f, FMath::Clamp(StaminaFraction, 0.0f, 1.0f));
	Mult *= StaminaPenalty;

	return FMath::Max(Mult, 0.05f); // never fully zero
}

// ─────────────────────────────────────────────────────────────────────────────
//  Recoil
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamWeaponPhysics::ApplyRecoilStep()
{
	bRecoveringRecoil = false;

	if (RecoilPattern.Pattern.Num() == 0) return;

	// Determine the index (wrap around using LoopStartIndex)
	int32 PatternIndex = CurrentRecoilIndex;
	if (PatternIndex >= RecoilPattern.Pattern.Num())
	{
		const int32 LoopStart = FMath::Clamp(RecoilPattern.LoopStartIndex, 0,
			RecoilPattern.Pattern.Num() - 1);
		PatternIndex = LoopStart +
			((PatternIndex - RecoilPattern.Pattern.Num()) %
				FMath::Max(1, RecoilPattern.Pattern.Num() - LoopStart));
	}

	FVector2D Step = RecoilPattern.Pattern[PatternIndex];

	// First-shot multiplier
	if (CurrentRecoilIndex == 0)
	{
		Step *= RecoilPattern.FirstShotMultiplier;
	}

	// Stability reduces recoil displacement
	const float Stability = ComputeStabilityMultiplier();
	Step *= Stability;

	AccumulatedRecoil += Step;
	CurrentRecoilIndex++;
}

void UBodyCamWeaponPhysics::BeginRecoilRecovery()
{
	bRecoveringRecoil = true;
}

void UBodyCamWeaponPhysics::ResetRecoil()
{
	CurrentRecoilIndex = 0;
	AccumulatedRecoil = FVector2D::ZeroVector;
	bRecoveringRecoil = false;
}

void UBodyCamWeaponPhysics::UpdateRecoilRecovery(float DeltaTime)
{
	if (!bRecoveringRecoil) return;
	if (AccumulatedRecoil.IsNearlyZero(0.01f))
	{
		ResetRecoil();
		return;
	}

	// Smoothly decay toward zero
	const float Rate = RecoilPattern.RecoveryRate * DeltaTime;
	AccumulatedRecoil = FMath::Vector2DInterpTo(AccumulatedRecoil, FVector2D::ZeroVector, DeltaTime, RecoilPattern.RecoveryRate);

	// Reset pattern index once recovered so next burst starts fresh
	if (AccumulatedRecoil.IsNearlyZero(0.05f))
	{
		ResetRecoil();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Weight-derived queries
// ─────────────────────────────────────────────────────────────────────────────

float UBodyCamWeaponPhysics::GetEffectiveADSTime() const
{
	// Heavier weapons take longer to ADS. Linear scaling from base.
	// Reference: 3.5 kg = BaseADSTime, each extra kg adds ~15%.
	const float WeightFactor = 1.0f + FMath::Max(0.0f, (WeaponWeight - 3.5f) * 0.15f);
	return BaseADSTime * WeightFactor;
}

float UBodyCamWeaponPhysics::GetMovementSpeedMultiplier() const
{
	// E.g. 3.5 kg weapon with 0.02 penalty/kg = 7% slower
	return FMath::Clamp(1.0f - (WeaponWeight * WeightSpeedPenaltyPerKg), 0.4f, 1.0f);
}

float UBodyCamWeaponPhysics::GetEffectiveLoudness() const
{
	return bHasSuppressor ? SuppressedLoudnessScale : 1.0f;
}

float UBodyCamWeaponPhysics::GetEffectiveMuzzleFlashScale() const
{
	return bHasSuppressor ? 0.0f : MuzzleFlashScale;
}
