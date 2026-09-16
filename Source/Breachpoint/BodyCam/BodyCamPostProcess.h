// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PostProcessVolume.h"
#include "BodyCamTypes.h"
#include "BodyCamPostProcess.generated.h"

class UCameraComponent;
class UTextRenderComponent;
class UUserWidget;
class UFont;

/**
 *  Applies body-cam–style post-processing to the owning actor's camera.
 *
 *  Attach this component to your VR pawn (or any actor with a UCameraComponent).
 *  It writes into the camera's PostProcessSettings every frame, so no
 *  post-process volume is required (though one can layer on top).
 *
 *  Features:
 *    - Film grain (subtle, increases in low light)
 *    - Chromatic aberration at screen edges
 *    - Vignette
 *    - Slight colour desaturation
 *    - Timestamp / "REC" overlay (via UMG widget)
 *    - Night-vision (green-tinted IR) toggle
 *    - Dynamic low-light noise
 */
UCLASS(ClassGroup = (BodyCam), meta = (BlueprintSpawnableComponent))
class BREACHPOINT_API UBodyCamPostProcess : public UActorComponent
{
	GENERATED_BODY()

public:

	UBodyCamPostProcess();

	// ── Lifecycle ───────────────────────────────────────────────────────

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// ── Settings ────────────────────────────────────────────────────────

	/** Master preset – all tunables in one struct for easy designer swapping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|Settings")
	FBodyCamSettings Settings;

	/** Overall intensity scaler (0 = no body-cam look, 1 = full effect). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|Settings",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterIntensity = 1.0f;

	/** When true, grain & noise respond to the scene's average luminance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|Settings")
	bool bDynamicLowLightNoise = true;

	/** Luminance threshold below which low-light noise kicks in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|Settings",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowLightThreshold = 0.25f;

	// ── Night Vision ────────────────────────────────────────────────────

	/** Current camera/overlay mode. */
	UPROPERTY(BlueprintReadOnly, Category = "BodyCam|NightVision")
	ECameraMode CurrentCameraMode = ECameraMode::Normal;

	/** Colour tint applied in night-vision mode (default: phosphor green). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|NightVision")
	FLinearColor NightVisionTint = FLinearColor(0.1f, 1.0f, 0.1f, 1.0f);

	/** Exposure bias boost for night-vision mode (EV). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|NightVision",
		meta = (ClampMin = "0.0", ClampMax = "8.0"))
	float NightVisionExposureBias = 3.5f;

	/** Extra grain added in NV mode to emulate sensor noise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|NightVision",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NightVisionGrainBoost = 0.25f;

	// ── Timestamp Overlay ───────────────────────────────────────────────

	/** Widget class for the timestamp/REC overlay (set in Blueprint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyCam|Overlay")
	TSubclassOf<UUserWidget> TimestampWidgetClass;

	// ── Blueprint API ───────────────────────────────────────────────────

	/** Toggle night-vision on/off. */
	UFUNCTION(BlueprintCallable, Category = "BodyCam|NightVision")
	void ToggleNightVision();

	/** Set a specific camera mode. */
	UFUNCTION(BlueprintCallable, Category = "BodyCam|NightVision")
	void SetCameraMode(ECameraMode NewMode);

	/** Manually override the estimated scene luminance (0–1). */
	UFUNCTION(BlueprintCallable, Category = "BodyCam|Settings")
	void SetSceneLuminanceOverride(float Luminance);

	/** Clear the luminance override so the component auto-detects again. */
	UFUNCTION(BlueprintCallable, Category = "BodyCam|Settings")
	void ClearSceneLuminanceOverride();

protected:

	// ── Internal helpers ────────────────────────────────────────────────

	/** Finds or caches the owning actor's camera component. */
	void CacheCamera();

	/** Writes all post-process values into the camera this frame. */
	void ApplyPostProcessSettings(float DeltaTime);

	/** Samples scene luminance (simplified: based on directional light or exposure). */
	float EstimateSceneLuminance() const;

	/** Creates the timestamp widget if a class is assigned. */
	void CreateTimestampWidget();

	/** Updates the widget text with the current real-world time + "REC". */
	void UpdateTimestampOverlay();

private:

	/** Cached reference to the owner's camera. */
	UPROPERTY()
	TObjectPtr<UCameraComponent> CachedCamera;

	/** Live timestamp widget instance. */
	UPROPERTY()
	TObjectPtr<UUserWidget> TimestampWidget;

	/** Optional manual luminance override (-1 = auto). */
	float LuminanceOverride = -1.0f;

	/** Smoothed luminance value to avoid popping. */
	float SmoothedLuminance = 0.5f;
};
