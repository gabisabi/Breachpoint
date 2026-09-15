// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "VRPlayerController.generated.h"

class UInputMappingContext;

/**
 * VR-specific player controller. Sets up VR input mapping contexts,
 * handles HMD tracking resets on respawn, and disables mobile touch controls.
 */
UCLASS(Abstract)
class BREACHPOINT_API AVRPlayerController : public AShooterPlayerController
{
	GENERATED_BODY()

public:
	AVRPlayerController();

	//~ Begin APlayerController Interface
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	//~ End APlayerController Interface

	/** Resets HMD orientation and position (e.g., on respawn). */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void ResetHMDTracking();

protected:
	/** VR-specific input mapping context applied at priority 0 (base layer). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputMappingContext> VRBaseMappingContext;

	/** Additional VR locomotion mapping context (smooth movement, teleport). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VR|Input")
	TObjectPtr<UInputMappingContext> VRLocomotionMappingContext;

	/** Priority for the base VR mapping context. */
	UPROPERTY(EditDefaultsOnly, Category = "VR|Input")
	int32 VRBaseMappingPriority = 0;

	/** Priority for the locomotion mapping context. */
	UPROPERTY(EditDefaultsOnly, Category = "VR|Input")
	int32 VRLocomotionMappingPriority = 1;

	/** Whether this controller is in VR mode. Set automatically from HMD availability. */
	UPROPERTY(BlueprintReadOnly, Category = "VR")
	bool bIsInVRMode = false;

	/** Adds VR mapping contexts to the Enhanced Input subsystem. */
	void SetupVRInputMappingContexts();

	/** Removes any mobile/touch control widgets (not applicable in VR). */
	void DisableMobileControls();

	/** Called when the pawn is respawned; resets HMD and re-applies input contexts. */
	virtual void HandleRespawn();
};
