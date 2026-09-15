// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/** Available crosshair visual styles */
UENUM(BlueprintType)
enum class ECrosshairStyle : uint8
{
	Default		UMETA(DisplayName="Default"),
	Dot			UMETA(DisplayName="Dot"),
	Circle		UMETA(DisplayName="Circle")
};

/**
 *  Dynamic crosshair widget that responds to movement and firing.
 *  Spread grows when AddSpread is called (on fire) and recovers
 *  toward MinSpread over time via NativeTick.
 *  Blueprint subclass is responsible for rendering the visual.
 */
UCLASS(abstract)
class BREACHPOINT_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ── Spread Parameters ─────────────────────────────────────

	/** Current spread value in pixels */
	UPROPERTY(BlueprintReadOnly, Category="Crosshair")
	float CurrentSpread = 5.0f;

	/** Maximum spread value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair", meta=(ClampMin=1, ClampMax=200))
	float MaxSpread = 50.0f;

	/** Minimum (resting) spread value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair", meta=(ClampMin=0, ClampMax=100))
	float MinSpread = 5.0f;

	/** Rate at which spread recovers toward MinSpread (pixels per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair", meta=(ClampMin=0, ClampMax=500))
	float SpreadRecoveryRate = 30.0f;

	/** Currently active crosshair style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
	ECrosshairStyle CrosshairStyle = ECrosshairStyle::Default;

	// ── Functions ─────────────────────────────────────────────

	/** Adds spread (typically called when the weapon fires) */
	UFUNCTION(BlueprintCallable, Category="Crosshair")
	void AddSpread(float Amount);

	/** Manually updates the spread recovery (called automatically via NativeTick) */
	UFUNCTION(BlueprintCallable, Category="Crosshair")
	void UpdateSpread(float DeltaTime);

	/** Sets the crosshair style */
	UFUNCTION(BlueprintCallable, Category="Crosshair")
	void SetCrosshairStyle(ECrosshairStyle NewStyle);

protected:

	/** Called every time the spread value changes so the Blueprint can update visuals */
	UFUNCTION(BlueprintImplementableEvent, Category="Crosshair", meta=(DisplayName="Update Crosshair"))
	void BP_UpdateCrosshair(float Spread);

	/** Called when the crosshair style changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Crosshair", meta=(DisplayName="On Style Changed"))
	void BP_OnStyleChanged(ECrosshairStyle NewStyle);

	/** Tick override for spread recovery */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
