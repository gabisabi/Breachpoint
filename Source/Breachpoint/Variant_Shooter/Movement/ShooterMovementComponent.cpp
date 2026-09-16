// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Movement/ShooterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"

UShooterMovementComponent::UShooterMovementComponent()
{
	// Enable ticking for smooth crouch interpolation
	PrimaryComponentTick.bCanEverTick = true;

	// Apply defaults
	MaxWalkSpeed = WalkSpeed;
	MaxWalkSpeedCrouched = CrouchSpeed;
	NavAgentProps.bCanCrouch = true;

	// Enable replication
	SetIsReplicatedByDefault(true);
}

// ── Lifecycle ───────────────────────────────────────────────────────────────

void UShooterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Store defaults
	MaxWalkSpeed = WalkSpeed;
	MaxWalkSpeedCrouched = CrouchSpeed;
	DefaultGroundFriction = GroundFriction;

	if (const ACharacter* Char = GetCharacterOwner())
	{
		DefaultCapsuleHalfHeight = Char->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	}
}

void UShooterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Smooth crouch camera transition
	if (ACharacter* Char = GetCharacterOwner())
	{
		const float TargetHalfHeight = IsCrouching() ? CrouchedHalfHeight : DefaultCapsuleHalfHeight;
		const float CurrentHalfHeight = Char->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

		if (!FMath::IsNearlyEqual(CurrentHalfHeight, TargetHalfHeight, 0.5f))
		{
			const float NewHalfHeight = FMath::FInterpTo(CurrentHalfHeight, TargetHalfHeight, DeltaTime, CrouchInterpSpeed);
			Char->GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);
		}
	}
}

// ── Speed override ──────────────────────────────────────────────────────────

float UShooterMovementComponent::GetMaxSpeed() const
{
	if (bIsSliding)
	{
		return SlideSpeed;
	}

	if (bIsTackling)
	{
		return TackleLungeSpeed;
	}

	if (IsSprinting())
	{
		return SprintSpeed;
	}

	return Super::GetMaxSpeed();
}

// ── Movement updated ────────────────────────────────────────────────────────

void UShooterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	// End slide if we've left the ground
	if (bIsSliding && !IsMovingOnGround())
	{
		EndSlide();
	}

	// Apply tackle damage during the lunge
	if (bIsTackling)
	{
		ApplyTackleDamage();
	}
}

// ── Sprint ──────────────────────────────────────────────────────────────────

void UShooterMovementComponent::StartSprint()
{
	bWantsToSprint = true;
}

void UShooterMovementComponent::StopSprint()
{
	bWantsToSprint = false;
}

// ── Slide ───────────────────────────────────────────────────────────────────

void UShooterMovementComponent::StartSlide()
{
	if (!bCanSlide || bIsSliding || !IsMovingOnGround() || IsCrouching())
	{
		return;
	}

	// Must have some forward velocity to slide
	if (!IsMovingForward())
	{
		return;
	}

	bIsSliding = true;
	bCanSlide = false;

	// Override ground friction for the slide
	GroundFriction = SlideGroundFriction;

	// Launch in current velocity direction with slide speed
	const FVector SlideDirection = Velocity.GetSafeNormal2D();
	Velocity = SlideDirection * SlideSpeed;

	// Lower camera
	if (ACharacter* Char = GetCharacterOwner())
	{
		Char->Crouch();
	}

	// Schedule slide end
	GetWorld()->GetTimerManager().SetTimer(SlideTimerHandle, this, &UShooterMovementComponent::OnSlideFinished, SlideDuration, false);
}

void UShooterMovementComponent::EndSlide()
{
	if (!bIsSliding)
	{
		return;
	}

	bIsSliding = false;

	// Restore ground friction
	GroundFriction = DefaultGroundFriction;

	// Stand back up
	if (ACharacter* Char = GetCharacterOwner())
	{
		Char->UnCrouch();
	}

	// Clear the duration timer if we ended early
	GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandle);

	// Start cooldown
	GetWorld()->GetTimerManager().SetTimer(SlideCooldownHandle, this, &UShooterMovementComponent::OnSlideCooldownFinished, SlideCooldown, false);
}

void UShooterMovementComponent::OnSlideFinished()
{
	EndSlide();
}

void UShooterMovementComponent::OnSlideCooldownFinished()
{
	bCanSlide = true;
}

// ── Tackle ──────────────────────────────────────────────────────────────────

void UShooterMovementComponent::StartTackle()
{
	if (!bCanTackle || bIsTackling || bIsSliding || !IsMovingOnGround())
	{
		return;
	}

	bIsTackling = true;
	bCanTackle = false;

	// Lunge forward
	const ACharacter* Char = GetCharacterOwner();
	if (Char)
	{
		const FVector Forward = Char->GetActorForwardVector();
		Velocity = Forward * TackleLungeSpeed;
	}

	// Duration based on distance / speed
	const float TackleDuration = TackleDistance / FMath::Max(1.f, TackleLungeSpeed);
	GetWorld()->GetTimerManager().SetTimer(TackleTimerHandle, this, &UShooterMovementComponent::OnTackleFinished, TackleDuration, false);
}

void UShooterMovementComponent::OnTackleFinished()
{
	bIsTackling = false;

	// Bleed off speed after tackle
	Velocity = Velocity.GetSafeNormal2D() * WalkSpeed;

	// Start cooldown
	GetWorld()->GetTimerManager().SetTimer(TackleCooldownHandle, this, &UShooterMovementComponent::OnTackleCooldownFinished, TackleCooldown, false);
}

void UShooterMovementComponent::OnTackleCooldownFinished()
{
	bCanTackle = true;
}

void UShooterMovementComponent::ApplyTackleDamage()
{
	ACharacter* Char = GetCharacterOwner();
	if (!Char)
	{
		return;
	}

	// Simple sphere overlap at character location
	const FVector Origin = Char->GetActorLocation();
	const float Radius = 100.0f;

	TArray<FHitResult> Hits;
	FCollisionShape Shape;
	Shape.SetSphere(Radius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	if (GetWorld()->SweepMultiByChannel(Hits, Origin, Origin + Char->GetActorForwardVector() * 10.0f, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != Char)
			{
				FDamageEvent DmgEvent;
				HitActor->TakeDamage(TackleDamage, DmgEvent, Char->GetController(), Char);

				// Only damage each actor once per tackle — end the tackle
				bIsTackling = false;
				GetWorld()->GetTimerManager().ClearTimer(TackleTimerHandle);
				Velocity = Velocity.GetSafeNormal2D() * WalkSpeed;

				// Start cooldown
				GetWorld()->GetTimerManager().SetTimer(TackleCooldownHandle, this, &UShooterMovementComponent::OnTackleCooldownFinished, TackleCooldown, false);
				return;
			}
		}
	}
}

// ── Helpers ─────────────────────────────────────────────────────────────────

bool UShooterMovementComponent::IsMovingForward() const
{
	if (const ACharacter* Char = GetCharacterOwner())
	{
		const FVector Forward = Char->GetActorForwardVector().GetSafeNormal2D();
		const FVector VelDir = Velocity.GetSafeNormal2D();
		return FVector::DotProduct(Forward, VelDir) > 0.5f;
	}
	return false;
}

// ── Replication ─────────────────────────────────────────────────────────────

void UShooterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShooterMovementComponent, bWantsToSprint);
	DOREPLIFETIME(UShooterMovementComponent, bIsSliding);
	DOREPLIFETIME(UShooterMovementComponent, bIsTackling);
}
