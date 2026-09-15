// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BreachpointHUD.generated.h"

/**
 *  Canvas-drawn HUD for Breachpoint game modes.
 *  Draws the objective line, wave/upgrade banners, the end-of-match screen
 *  and the game mode selection menu.
 */
UCLASS()
class BREACHPOINT_API ABreachpointHUD : public AHUD
{
	GENERATED_BODY()

public:

	/** If true, the game mode selection menu is drawn */
	bool bMenuOpen = false;

	/** Main draw call */
	virtual void DrawHUD() override;

protected:

	/** Draws a horizontally centered string. Returns the height used */
	float DrawCenteredText(const FString& Text, UFont* Font, float Scale, float Y, const FLinearColor& Color);

	/** Draws a full-width translucent panel */
	void DrawPanel(float Y, float Height, float Alpha = 0.55f);
};
