// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BreachpointHUD.generated.h"

/** Lobby sub-screen */
UENUM()
enum class ELobbyScreen : uint8
{
	None,				// Not in lobby — show in-game HUD
	MainMenu,			// Title + Play / Settings / Quit
	ModeSelect,			// Pick a game mode
	CharacterSelect		// Pick an operator
};

/**
 *  Canvas-drawn HUD for Breachpoint.
 *  STATE 1 — Multi-screen lobby (MainMenu → ModeSelect → CharacterSelect → Start)
 *  STATE 2 — In-game HUD (health, weapons, crosshair, objective, banners, end screen)
 */
UCLASS()
class BREACHPOINT_API ABreachpointHUD : public AHUD
{
	GENERATED_BODY()

public:

	// ── Lobby state ──────────────────────────────────────────────

	/** Current lobby screen (None = in-game) */
	ELobbyScreen CurrentScreen = ELobbyScreen::None;

	/** Highlighted item index for whichever screen is active */
	int32 SelectedIndex = 0;

	/** Which game mode was selected (index into mode list) */
	int32 SelectedModeIndex = 0;

	/** Which operator was selected (index into operator list) */
	int32 SelectedOperatorIndex = 0;

	// ── In-game pause overlay ────────────────────────────────────

	/** If true, the in-game pause overlay is shown */
	bool bPauseMenuOpen = false;

	int32 PauseSelectedIndex = 0;

	// ── Main draw call ───────────────────────────────────────────

	virtual void DrawHUD() override;

	// ── Lobby navigation (called by controller) ──────────────────

	/** Move selection up */
	void NavigateUp();

	/** Move selection down */
	void NavigateDown();

	/** Move selection left */
	void NavigateLeft();

	/** Move selection right */
	void NavigateRight();

	/** Confirm the current selection */
	void Confirm();

	/** Go back to the previous screen */
	void GoBack();

protected:

	// ── Lobby screens ────────────────────────────────────────────

	void DrawMainMenu(float SizeX, float SizeY);
	void DrawModeSelect(float SizeX, float SizeY);
	void DrawCharacterSelect(float SizeX, float SizeY);

	// ── In-game HUD elements ─────────────────────────────────────

	void DrawHealthBar(float SizeX, float SizeY);
	void DrawWeaponInventory(float SizeX, float SizeY);
	void DrawCrosshair(float SizeX, float SizeY);
	void DrawObjectiveAndMode(float SizeX, float SizeY);
	void DrawBanner(float SizeX, float SizeY);
	void DrawEndScreen(float SizeX, float SizeY);
	void DrawPauseMenu(float SizeX, float SizeY);

	// ── Shared helpers ───────────────────────────────────────────

	/** Draws a horizontally centered string. Returns the height used */
	float DrawCenteredText(const FString& Text, UFont* Font, float Scale, float Y, const FLinearColor& Color);

	/** Draws a full-width translucent panel */
	void DrawPanel(float Y, float Height, float Alpha = 0.55f);

	/** Draws a text label at a position with optional shadow */
	void DrawShadowedText(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Color);

	/** Starts the game from the lobby */
	void LaunchGame();
};
