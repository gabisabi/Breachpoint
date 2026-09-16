// Copyright Breachpoint. All Rights Reserved.

#include "BodyCamPostProcess.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"

UBodyCamPostProcess::UBodyCamPostProcess()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // after movement, before render
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamPostProcess::BeginPlay()
{
	Super::BeginPlay();
	CacheCamera();
	CreateTimestampWidget();
}

void UBodyCamPostProcess::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedCamera)
	{
		CacheCamera();
		if (!CachedCamera) return;
	}

	ApplyPostProcessSettings(DeltaTime);
	UpdateTimestampOverlay();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Camera caching
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamPostProcess::CacheCamera()
{
	if (AActor* Owner = GetOwner())
	{
		CachedCamera = Owner->FindComponentByClass<UCameraComponent>();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Post-process application
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamPostProcess::ApplyPostProcessSettings(float DeltaTime)
{
	if (!CachedCamera) return;

	FPostProcessSettings& PP = CachedCamera->PostProcessSettings;
	CachedCamera->PostProcessBlendWeight = 1.0f; // layer on top

	const float Alpha = MasterIntensity;

	// --- Film grain ---
	float GrainAmount = Settings.FilmGrainIntensity * Alpha;

	// Dynamic low-light boost
	if (bDynamicLowLightNoise)
	{
		const float RawLuminance = (LuminanceOverride >= 0.0f)
			? LuminanceOverride
			: EstimateSceneLuminance();

		// Smooth the luminance to avoid flicker
		SmoothedLuminance = FMath::FInterpTo(SmoothedLuminance, RawLuminance, DeltaTime, 3.0f);

		if (SmoothedLuminance < LowLightThreshold)
		{
			const float T = 1.0f - (SmoothedLuminance / FMath::Max(LowLightThreshold, 0.001f));
			GrainAmount += Settings.LowLightNoiseGain * T * Alpha;
		}
	}

	// Night-vision grain boost
	if (CurrentCameraMode == ECameraMode::NightVision)
	{
		GrainAmount += NightVisionGrainBoost * Alpha;
	}

	PP.bOverride_FilmGrainIntensity = true;
	PP.FilmGrainIntensity = FMath::Clamp(GrainAmount, 0.0f, 1.0f);

	// --- Chromatic aberration ---
	PP.bOverride_SceneFringeIntensity = true;
	PP.SceneFringeIntensity = Settings.ChromaticAberrationIntensity * Alpha;

	// --- Vignette ---
	PP.bOverride_VignetteIntensity = true;
	PP.VignetteIntensity = Settings.VignetteIntensity * Alpha;

	// --- Desaturation ---
	PP.bOverride_ColorSaturation = true;
	const float Sat = 1.0f - (Settings.DesaturationAmount * Alpha);
	PP.ColorSaturation = FVector4(Sat, Sat, Sat, 1.0f);

	// --- Night-vision tint + exposure ---
	if (CurrentCameraMode == ECameraMode::NightVision)
	{
		PP.bOverride_ColorGain = true;
		PP.ColorGain = FVector4(NightVisionTint.R, NightVisionTint.G, NightVisionTint.B, 1.0f);

		PP.bOverride_AutoExposureBias = true;
		PP.AutoExposureBias = NightVisionExposureBias;

		// Extra desaturation for the IR monochrome look
		PP.ColorSaturation = FVector4(0.15f, 0.15f, 0.15f, 1.0f);
	}
	else
	{
		PP.bOverride_ColorGain = false;
		PP.bOverride_AutoExposureBias = false;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Scene luminance estimation
// ─────────────────────────────────────────────────────────────────────────────

float UBodyCamPostProcess::EstimateSceneLuminance() const
{
	// Simple heuristic: find the dominant directional light and use its intensity
	// as a proxy for scene brightness. A proper implementation would read back
	// the auto-exposure luminance from the renderer, but that requires a
	// scene-view extension (TODO: hook into FSceneViewExtensionBase).

	UWorld* World = GetWorld();
	if (!World) return 0.5f;

	float MaxIntensity = 0.0f;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		if (ULightComponent* LC = It->GetLightComponent())
		{
			MaxIntensity = FMath::Max(MaxIntensity, LC->Intensity);
		}
	}

	// Normalise: typical UE directional light intensity is 1–10 lux
	return FMath::Clamp(MaxIntensity / 10.0f, 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Night-vision toggle
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamPostProcess::ToggleNightVision()
{
	SetCameraMode(CurrentCameraMode == ECameraMode::NightVision
		? ECameraMode::Normal
		: ECameraMode::NightVision);
}

void UBodyCamPostProcess::SetCameraMode(ECameraMode NewMode)
{
	CurrentCameraMode = NewMode;
	// TODO: play NV on/off sound cue, animate transition (quick green flash)
}

// ─────────────────────────────────────────────────────────────────────────────
//  Luminance override
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamPostProcess::SetSceneLuminanceOverride(float Luminance)
{
	LuminanceOverride = FMath::Clamp(Luminance, 0.0f, 1.0f);
}

void UBodyCamPostProcess::ClearSceneLuminanceOverride()
{
	LuminanceOverride = -1.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Timestamp / REC overlay
// ─────────────────────────────────────────────────────────────────────────────

void UBodyCamPostProcess::CreateTimestampWidget()
{
	if (!TimestampWidgetClass) return;

	APlayerController* PC = GetWorld()
		? GetWorld()->GetFirstPlayerController()
		: nullptr;
	if (!PC) return;

	TimestampWidget = CreateWidget<UUserWidget>(PC, TimestampWidgetClass);
	if (TimestampWidget)
	{
		TimestampWidget->AddToViewport(100); // high Z-order
	}
}

void UBodyCamPostProcess::UpdateTimestampOverlay()
{
	// The actual text binding is done via UMG bindings in Blueprint.
	// This function can push data to the widget if needed (e.g. via interface).
	//
	// Designers should create a WBP_BodyCamOverlay with:
	//   - Top-left: "● REC" blinking text (red dot)
	//   - Bottom-right: real-time timestamp (FDateTime::Now formatted)
	//   - Optional: battery icon, unit ID, cam number
	//
	// For C++ driven text, expose a UFUNCTION in the widget and call here:
	//   if (TimestampWidget && TimestampWidget->Implements<UBodyCamOverlayInterface>())
	//       IBodyCamOverlayInterface::Execute_SetTimestamp(TimestampWidget, FDateTime::Now().ToString());
	//
	// TODO: implement UBodyCamOverlayInterface for direct C++ → widget text push
}
