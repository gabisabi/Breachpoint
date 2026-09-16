// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Shooter/Modes/BreachpointHUD.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Colour palette
// ═══════════════════════════════════════════════════════════════════════════════
namespace
{
	const FLinearColor Accent(1.0f, 0.62f, 0.11f);       // orange highlight
	const FLinearColor White(0.95f, 0.97f, 1.0f);
	const FLinearColor Dim(0.55f, 0.58f, 0.62f);
	const FLinearColor DarkBG(0.02f, 0.04f, 0.07f, 0.92f);
	const FLinearColor PanelBG(0.05f, 0.08f, 0.12f, 0.85f);
	const FLinearColor HealthRed(0.85f, 0.12f, 0.12f);
	const FLinearColor HealthBG(0.2f, 0.2f, 0.2f, 0.7f);
	const FLinearColor SlotBG(0.08f, 0.10f, 0.14f, 0.8f);
	const FLinearColor SlotActive(1.0f, 0.62f, 0.11f, 0.35f);

	// Mode definitions (kept in sync with LobbyGameMode::PopulateDefaultGameModes)
	struct FModeEntry
	{
		const TCHAR* Name;
		const TCHAR* Desc;
		const TCHAR* Options;
	};

	const FModeEntry Modes[] = {
		{ TEXT("FREE FOR ALL"),      TEXT("Every player for themselves"),      TEXT("") },
		{ TEXT("HORDE"),             TEXT("Survive enemy waves"),              TEXT("?game=/Script/Breachpoint.HordeGameMode") },
		{ TEXT("GUN GAME"),          TEXT("Kill to upgrade weapons"),          TEXT("?game=/Script/Breachpoint.GunGameMode") },
		{ TEXT("TEAM DEATHMATCH"),   TEXT("First to 15 wins"),                TEXT("?game=/Script/Breachpoint.TeamDeathmatchMode") },
	};
	constexpr int32 NumModes = UE_ARRAY_COUNT(Modes);

	// Operator placeholders
	const TCHAR* Operators[] = { TEXT("OPERATOR 1"), TEXT("OPERATOR 2"), TEXT("OPERATOR 3") };
	constexpr int32 NumOperators = UE_ARRAY_COUNT(Operators);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Main draw call — routes to lobby or in-game HUD
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float SizeX = Canvas->SizeX;
	const float SizeY = Canvas->SizeY;

	switch (CurrentScreen)
	{
	case ELobbyScreen::MainMenu:
		DrawRect(DarkBG, 0.0f, 0.0f, SizeX, SizeY);
		DrawMainMenu(SizeX, SizeY);
		return;

	case ELobbyScreen::ModeSelect:
		DrawRect(DarkBG, 0.0f, 0.0f, SizeX, SizeY);
		DrawModeSelect(SizeX, SizeY);
		return;

	case ELobbyScreen::CharacterSelect:
		DrawRect(DarkBG, 0.0f, 0.0f, SizeX, SizeY);
		DrawCharacterSelect(SizeX, SizeY);
		return;

	default:
		break;
	}

	// ── In-game HUD ─────────────────────────────────────────────
	DrawHealthBar(SizeX, SizeY);
	DrawObjectiveAndMode(SizeX, SizeY);
	DrawWeaponInventory(SizeX, SizeY);
	DrawCrosshair(SizeX, SizeY);
	DrawBanner(SizeX, SizeY);
	DrawEndScreen(SizeX, SizeY);

	if (bPauseMenuOpen)
	{
		DrawPauseMenu(SizeX, SizeY);
	}
}

// ═══════════════════════════════════════════════════════════════════════════════
// LOBBY SCREEN 1 — Main Menu
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawMainMenu(float SizeX, float SizeY)
{
	UFont* Large = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	const float TitleScale = FMath::Max(3.5f, SizeY / 250.0f);
	const float ItemScale = FMath::Max(1.6f, SizeY / 450.0f);

	// Title
	DrawCenteredText(TEXT("BREACHPOINT"), Large, TitleScale, SizeY * 0.15f, Accent);

	// Decorative line under title
	const float LineW = SizeX * 0.3f;
	DrawRect(Accent, (SizeX - LineW) * 0.5f, SizeY * 0.15f + TitleScale * 28.0f, LineW, 3.0f);

	// Menu items
	const TCHAR* Items[] = { TEXT("PLAY"), TEXT("SETTINGS"), TEXT("QUIT") };
	const float StartY = SizeY * 0.42f;
	const float Spacing = SizeY * 0.08f;

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bSelected = (i == SelectedIndex);
		const bool bDisabled = (i == 1); // Settings not implemented

		FLinearColor Color = bSelected ? Accent : (bDisabled ? Dim : White);

		// Selection indicator
		if (bSelected)
		{
			float W = 0.0f, H = 0.0f;
			Canvas->TextSize(Medium, Items[i], W, H, ItemScale, ItemScale);
			const float BoxW = W + 40.0f;
			const float BoxH = H + 16.0f;
			DrawRect(FLinearColor(Accent.R, Accent.G, Accent.B, 0.15f),
				(SizeX - BoxW) * 0.5f, StartY + i * Spacing - 8.0f, BoxW, BoxH);

			// Arrow indicator
			DrawShadowedText(TEXT(">"), (SizeX - BoxW) * 0.5f - 24.0f, StartY + i * Spacing, Medium, ItemScale, Accent);
		}

		DrawCenteredText(Items[i], Medium, ItemScale, StartY + i * Spacing, Color);
	}

	// Footer
	DrawCenteredText(TEXT("UP / DOWN  -  NAVIGATE        ENTER  -  SELECT"), Medium, FMath::Max(0.9f, SizeY / 800.0f), SizeY * 0.88f, Dim);
}

// ═══════════════════════════════════════════════════════════════════════════════
// LOBBY SCREEN 2 — Game Mode Select
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawModeSelect(float SizeX, float SizeY)
{
	UFont* Large = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	const float TitleScale = FMath::Max(2.5f, SizeY / 320.0f);
	const float NameScale = FMath::Max(1.4f, SizeY / 500.0f);
	const float DescScale = FMath::Max(1.0f, SizeY / 650.0f);

	// Title
	DrawCenteredText(TEXT("SELECT MODE"), Large, TitleScale, SizeY * 0.1f, Accent);

	// Mode list
	const float StartY = SizeY * 0.28f;
	const float SlotH = SizeY * 0.11f;
	const float SlotW = SizeX * 0.5f;
	const float SlotX = (SizeX - SlotW) * 0.5f;

	for (int32 i = 0; i < NumModes; ++i)
	{
		const float Y = StartY + i * (SlotH + SizeY * 0.02f);
		const bool bSelected = (i == SelectedIndex);

		// Slot background
		DrawRect(bSelected ? SlotActive : SlotBG, SlotX, Y, SlotW, SlotH);

		// Left accent bar for selected
		if (bSelected)
		{
			DrawRect(Accent, SlotX, Y, 4.0f, SlotH);
		}

		// Mode number
		FString NumStr = FString::Printf(TEXT("%d"), i + 1);
		DrawShadowedText(NumStr, SlotX + 16.0f, Y + SlotH * 0.15f, Large, NameScale, bSelected ? Accent : Dim);

		// Mode name
		DrawShadowedText(Modes[i].Name, SlotX + 60.0f, Y + SlotH * 0.12f, Medium, NameScale, bSelected ? Accent : White);

		// Mode description
		DrawShadowedText(Modes[i].Desc, SlotX + 60.0f, Y + SlotH * 0.55f, Medium, DescScale, Dim);
	}

	// Footer
	const float FootScale = FMath::Max(0.9f, SizeY / 800.0f);
	DrawCenteredText(TEXT("UP / DOWN  -  NAVIGATE        ENTER  -  SELECT        ESC  -  BACK"), Medium, FootScale, SizeY * 0.88f, Dim);
}

// ═══════════════════════════════════════════════════════════════════════════════
// LOBBY SCREEN 3 — Character / Operator Select
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawCharacterSelect(float SizeX, float SizeY)
{
	UFont* Large = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	const float TitleScale = FMath::Max(2.5f, SizeY / 320.0f);
	const float NameScale = FMath::Max(1.6f, SizeY / 420.0f);

	// Title
	DrawCenteredText(TEXT("SELECT OPERATOR"), Large, TitleScale, SizeY * 0.1f, Accent);

	// Operator cards — horizontal layout
	const float CardW = SizeX * 0.18f;
	const float CardH = SizeY * 0.35f;
	const float Gap = SizeX * 0.04f;
	const float TotalW = NumOperators * CardW + (NumOperators - 1) * Gap;
	const float StartX = (SizeX - TotalW) * 0.5f;
	const float CardY = SizeY * 0.3f;

	for (int32 i = 0; i < NumOperators; ++i)
	{
		const float X = StartX + i * (CardW + Gap);
		const bool bSelected = (i == SelectedIndex);

		// Card background
		DrawRect(bSelected ? SlotActive : SlotBG, X, CardY, CardW, CardH);

		// Border for selected
		if (bSelected)
		{
			const float B = 3.0f;
			DrawRect(Accent, X, CardY, CardW, B);                     // top
			DrawRect(Accent, X, CardY + CardH - B, CardW, B);         // bottom
			DrawRect(Accent, X, CardY, B, CardH);                     // left
			DrawRect(Accent, X + CardW - B, CardY, B, CardH);         // right
		}

		// Operator silhouette placeholder (centered in card)
		const float IconSize = FMath::Min(CardW, CardH) * 0.4f;
		DrawRect(FLinearColor(0.15f, 0.18f, 0.22f, 0.6f),
			X + (CardW - IconSize) * 0.5f, CardY + CardH * 0.15f, IconSize, IconSize);

		// Operator name centered below icon
		float W = 0.0f, H = 0.0f;
		Canvas->TextSize(Medium, Operators[i], W, H, NameScale * 0.7f, NameScale * 0.7f);
		DrawShadowedText(Operators[i], X + (CardW - W) * 0.5f, CardY + CardH * 0.7f, Medium, NameScale * 0.7f, bSelected ? Accent : White);
	}

	// Left / Right arrows
	const float ArrowScale = FMath::Max(2.5f, SizeY / 300.0f);
	DrawShadowedText(TEXT("<"), StartX - 50.0f, CardY + CardH * 0.4f, Large, ArrowScale, Dim);
	DrawShadowedText(TEXT(">"), StartX + TotalW + 20.0f, CardY + CardH * 0.4f, Large, ArrowScale, Dim);

	// "PRESS ENTER TO START"
	const float BottomScale = FMath::Max(1.4f, SizeY / 480.0f);
	DrawCenteredText(TEXT("PRESS ENTER TO START"), Medium, BottomScale, SizeY * 0.78f, White);

	// Footer
	const float FootScale = FMath::Max(0.9f, SizeY / 800.0f);
	DrawCenteredText(TEXT("LEFT / RIGHT  -  CHOOSE        ENTER  -  START        ESC  -  BACK"), Medium, FootScale, SizeY * 0.88f, Dim);
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME HUD — Health bar (top left)
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawHealthBar(float SizeX, float SizeY)
{
	AShooterCharacter* Char = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!Char)
	{
		return;
	}

	UFont* Medium = GEngine->GetMediumFont();
	const float Scale = FMath::Max(1.0f, SizeY / 650.0f);

	const float BarX = SizeX * 0.02f;
	const float BarY = SizeY * 0.03f;
	const float BarW = SizeX * 0.18f;
	const float BarH = SizeY * 0.025f;

	const float HPPct = (Char->GetMaxHP() > 0.0f) ? FMath::Clamp(Char->GetCurrentHP() / Char->GetMaxHP(), 0.0f, 1.0f) : 0.0f;

	// Background
	DrawRect(HealthBG, BarX, BarY, BarW, BarH);

	// Health fill
	const FLinearColor FillColor = FMath::Lerp(FLinearColor(0.85f, 0.1f, 0.1f), FLinearColor(0.1f, 0.85f, 0.2f), HPPct);
	DrawRect(FillColor, BarX, BarY, BarW * HPPct, BarH);

	// Border
	DrawRect(FLinearColor(0.6f, 0.6f, 0.6f, 0.5f), BarX, BarY, BarW, 1.0f);
	DrawRect(FLinearColor(0.6f, 0.6f, 0.6f, 0.5f), BarX, BarY + BarH, BarW, 1.0f);

	// HP text
	const FString HPText = FString::Printf(TEXT("HP  %d / %d"), FMath::CeilToInt(Char->GetCurrentHP()), FMath::CeilToInt(Char->GetMaxHP()));
	DrawShadowedText(HPText, BarX, BarY + BarH + 4.0f, Medium, Scale * 0.85f, White);
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME HUD — Weapon inventory (bottom right)
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawWeaponInventory(float SizeX, float SizeY)
{
	AShooterCharacter* Char = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!Char)
	{
		return;
	}

	UFont* Medium = GEngine->GetMediumFont();
	const float Scale = FMath::Max(1.0f, SizeY / 650.0f);

	const TArray<AShooterWeapon*>& Weapons = Char->GetOwnedWeapons();
	AShooterWeapon* Active = Char->GetCurrentWeapon();

	const float SlotW = SizeX * 0.14f;
	const float SlotH = SizeY * 0.065f;
	const float Margin = SizeX * 0.015f;
	const float Gap = SizeY * 0.008f;

	const float BaseX = SizeX - SlotW - Margin;
	float Y = SizeY - Margin - (Weapons.Num() * (SlotH + Gap));

	for (int32 i = 0; i < Weapons.Num(); ++i)
	{
		AShooterWeapon* Wep = Weapons[i];
		if (!IsValid(Wep))
		{
			Y += SlotH + Gap;
			continue;
		}

		const bool bActive = (Wep == Active);

		// Slot bg
		DrawRect(bActive ? SlotActive : SlotBG, BaseX, Y, SlotW, SlotH);

		// Active indicator bar
		if (bActive)
		{
			DrawRect(Accent, BaseX, Y, 3.0f, SlotH);
		}

		// Weapon name — use class name as display name
		FString WepName = Wep->GetClass()->GetName();
		WepName.RemoveFromStart(TEXT("BP_"));
		WepName.RemoveFromEnd(TEXT("_C"));
		DrawShadowedText(WepName, BaseX + 10.0f, Y + 4.0f, Medium, Scale * 0.75f, bActive ? Accent : White);

		// Ammo display
		const FString AmmoText = FString::Printf(TEXT("%d / %d"), Wep->GetBulletCount(), Wep->GetMagazineSize());
		DrawShadowedText(AmmoText, BaseX + 10.0f, Y + SlotH * 0.5f, Medium, Scale * 0.7f, Dim);

		Y += SlotH + Gap;
	}

	// If we have a current weapon, also show larger ammo text at bottom right
	if (IsValid(Active))
	{
		const float BigScale = FMath::Max(1.6f, SizeY / 400.0f);
		const FString BigAmmo = FString::Printf(TEXT("%d / %d"), Active->GetBulletCount(), Active->GetMagazineSize());
		float W = 0.0f, H = 0.0f;
		Canvas->TextSize(Medium, BigAmmo, W, H, BigScale, BigScale);
		DrawShadowedText(BigAmmo, SizeX - W - Margin, SizeY - H - Margin * 0.5f, Medium, BigScale, White);
	}
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME HUD — Crosshair (center)
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawCrosshair(float SizeX, float SizeY)
{
	const float CX = SizeX * 0.5f;
	const float CY = SizeY * 0.5f;
	const float Arm = FMath::Max(8.0f, SizeY * 0.012f);
	const float Thick = FMath::Max(2.0f, SizeY * 0.002f);
	const float GapSize = FMath::Max(3.0f, SizeY * 0.004f);

	const FLinearColor CrossColor(1.0f, 1.0f, 1.0f, 0.85f);

	// Horizontal arms
	DrawRect(CrossColor, CX - Arm - GapSize, CY - Thick * 0.5f, Arm, Thick);
	DrawRect(CrossColor, CX + GapSize, CY - Thick * 0.5f, Arm, Thick);

	// Vertical arms
	DrawRect(CrossColor, CX - Thick * 0.5f, CY - Arm - GapSize, Thick, Arm);
	DrawRect(CrossColor, CX - Thick * 0.5f, CY + GapSize, Thick, Arm);

	// Center dot
	DrawRect(CrossColor, CX - Thick * 0.5f, CY - Thick * 0.5f, Thick, Thick);
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME HUD — Objective and mode name (top right)
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawObjectiveAndMode(float SizeX, float SizeY)
{
	ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!Mode)
	{
		return;
	}

	UFont* Medium = GEngine->GetMediumFont();
	const float Scale = FMath::Max(1.0f, SizeY / 650.0f);
	const float Margin = SizeX * 0.015f;
	float Y = SizeY * 0.03f;

	// Mode name (use class name, stripped)
	FString ModeName = Mode->GetClass()->GetName();
	ModeName.RemoveFromStart(TEXT("A"));
	ModeName.RemoveFromEnd(TEXT("_C"));

	float W = 0.0f, H = 0.0f;
	Canvas->TextSize(Medium, ModeName, W, H, Scale, Scale);
	DrawShadowedText(ModeName, SizeX - W - Margin, Y, Medium, Scale, Accent);
	Y += H + 4.0f;

	// Objective text
	if (!Mode->GetObjectiveText().IsEmpty())
	{
		Canvas->TextSize(Medium, Mode->GetObjectiveText(), W, H, Scale * 0.85f, Scale * 0.85f);
		DrawShadowedText(Mode->GetObjectiveText(), SizeX - W - Margin, Y, Medium, Scale * 0.85f, White);
	}
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME HUD — Center banner (wave announcements etc.)
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawBanner(float SizeX, float SizeY)
{
	ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!Mode || Mode->IsMatchEnded())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() < Mode->GetBannerUntil() && !Mode->GetBannerText().IsEmpty())
	{
		UFont* Large = GEngine->GetLargeFont();
		const float BigScale = FMath::Max(2.0f, SizeY / 360.0f);
		DrawCenteredText(Mode->GetBannerText(), Large, BigScale, SizeY * 0.28f, Accent);
	}
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME HUD — End-of-match screen
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawEndScreen(float SizeX, float SizeY)
{
	ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!Mode || !Mode->IsMatchEnded())
	{
		return;
	}

	UFont* Large = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	const float BigScale = FMath::Max(2.0f, SizeY / 360.0f);
	const float MidScale = FMath::Max(1.25f, SizeY / 600.0f);

	DrawPanel(SizeY * 0.3f, SizeY * 0.28f, 0.6f);
	float Y = SizeY * 0.34f;
	Y += DrawCenteredText(Mode->GetEndText(), Large, BigScale, Y, Accent) + SizeY * 0.03f;
	DrawCenteredText(TEXT("ENTER - restart    TAB - return to lobby"), Medium, MidScale, Y, White);
}

// ═══════════════════════════════════════════════════════════════════════════════
// IN-GAME — Pause menu overlay
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::DrawPauseMenu(float SizeX, float SizeY)
{
	UFont* Large = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();
	const float TitleScale = FMath::Max(2.0f, SizeY / 360.0f);
	const float ItemScale = FMath::Max(1.3f, SizeY / 520.0f);

	// Dim overlay
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), 0.0f, 0.0f, SizeX, SizeY);

	DrawCenteredText(TEXT("PAUSED"), Large, TitleScale, SizeY * 0.3f, Accent);

	const TCHAR* Items[] = { TEXT("RESUME"), TEXT("QUIT TO LOBBY") };
	const float StartY = SizeY * 0.45f;

	for (int32 i = 0; i < 2; ++i)
	{
		const bool bSel = (i == PauseSelectedIndex);
		DrawCenteredText(Items[i], Medium, ItemScale, StartY + i * SizeY * 0.07f, bSel ? Accent : White);
	}

	DrawCenteredText(TEXT("UP / DOWN  -  NAVIGATE        ENTER  -  SELECT"), Medium, FMath::Max(0.9f, SizeY / 800.0f), SizeY * 0.88f, Dim);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Lobby navigation
// ═══════════════════════════════════════════════════════════════════════════════
void ABreachpointHUD::NavigateUp()
{
	if (CurrentScreen == ELobbyScreen::MainMenu)
	{
		SelectedIndex = (SelectedIndex - 1 + 3) % 3;
	}
	else if (CurrentScreen == ELobbyScreen::ModeSelect)
	{
		SelectedIndex = (SelectedIndex - 1 + NumModes) % NumModes;
	}
	else if (CurrentScreen == ELobbyScreen::None && bPauseMenuOpen)
	{
		PauseSelectedIndex = (PauseSelectedIndex - 1 + 2) % 2;
	}
}

void ABreachpointHUD::NavigateDown()
{
	if (CurrentScreen == ELobbyScreen::MainMenu)
	{
		SelectedIndex = (SelectedIndex + 1) % 3;
	}
	else if (CurrentScreen == ELobbyScreen::ModeSelect)
	{
		SelectedIndex = (SelectedIndex + 1) % NumModes;
	}
	else if (CurrentScreen == ELobbyScreen::None && bPauseMenuOpen)
	{
		PauseSelectedIndex = (PauseSelectedIndex + 1) % 2;
	}
}

void ABreachpointHUD::NavigateLeft()
{
	if (CurrentScreen == ELobbyScreen::CharacterSelect)
	{
		SelectedIndex = (SelectedIndex - 1 + NumOperators) % NumOperators;
	}
}

void ABreachpointHUD::NavigateRight()
{
	if (CurrentScreen == ELobbyScreen::CharacterSelect)
	{
		SelectedIndex = (SelectedIndex + 1) % NumOperators;
	}
}

void ABreachpointHUD::Confirm()
{
	switch (CurrentScreen)
	{
	case ELobbyScreen::MainMenu:
		if (SelectedIndex == 0) // PLAY
		{
			CurrentScreen = ELobbyScreen::ModeSelect;
			SelectedIndex = SelectedModeIndex; // restore previous selection
		}
		else if (SelectedIndex == 2) // QUIT
		{
			if (APlayerController* PC = GetOwningPlayerController())
			{
				PC->ConsoleCommand(TEXT("quit"));
			}
		}
		// Settings (1) — do nothing for now
		break;

	case ELobbyScreen::ModeSelect:
		SelectedModeIndex = SelectedIndex;
		CurrentScreen = ELobbyScreen::CharacterSelect;
		SelectedIndex = SelectedOperatorIndex; // restore previous selection
		break;

	case ELobbyScreen::CharacterSelect:
		SelectedOperatorIndex = SelectedIndex;
		LaunchGame();
		break;

	case ELobbyScreen::None:
		if (bPauseMenuOpen)
		{
			if (PauseSelectedIndex == 0)
			{
				// Resume
				bPauseMenuOpen = false;
			}
			else
			{
				// Quit to lobby — reload level with lobby game mode
				UGameplayStatics::OpenLevel(this,
					FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true,
					TEXT("?game=/Script/Breachpoint.LobbyGameMode"));
			}
		}
		else
		{
			// Handle end-of-match restart
			ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(GetWorld()->GetAuthGameMode());
			if (Mode && Mode->IsMatchEnded())
			{
				Mode->RestartMatch();
			}
		}
		break;

	default:
		break;
	}
}

void ABreachpointHUD::GoBack()
{
	switch (CurrentScreen)
	{
	case ELobbyScreen::ModeSelect:
		CurrentScreen = ELobbyScreen::MainMenu;
		SelectedIndex = 0; // "PLAY"
		break;

	case ELobbyScreen::CharacterSelect:
		CurrentScreen = ELobbyScreen::ModeSelect;
		SelectedIndex = SelectedModeIndex;
		break;

	case ELobbyScreen::None:
		// Toggle pause menu
		bPauseMenuOpen = !bPauseMenuOpen;
		PauseSelectedIndex = 0;
		break;

	default:
		break;
	}
}

void ABreachpointHUD::LaunchGame()
{
	if (SelectedModeIndex < 0 || SelectedModeIndex >= NumModes)
	{
		return;
	}

	UGameplayStatics::OpenLevel(this,
		FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true,
		Modes[SelectedModeIndex].Options);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Shared helpers
// ═══════════════════════════════════════════════════════════════════════════════
float ABreachpointHUD::DrawCenteredText(const FString& Text, UFont* Font, float Scale, float Y, const FLinearColor& Color)
{
	float W = 0.0f, H = 0.0f;
	Canvas->TextSize(Font, Text, W, H, Scale, Scale);

	const float X = (Canvas->SizeX - W) * 0.5f;

	// Soft shadow
	DrawText(Text, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), X + 2.0f, Y + 2.0f, Font, Scale);
	DrawText(Text, Color, X, Y, Font, Scale);

	return H;
}

void ABreachpointHUD::DrawPanel(float Y, float Height, float Alpha)
{
	DrawRect(FLinearColor(0.02f, 0.05f, 0.09f, Alpha), 0.0f, Y, Canvas->SizeX, Height);
	DrawRect(Accent, 0.0f, Y, Canvas->SizeX, 3.0f);
	DrawRect(Accent, 0.0f, Y + Height - 3.0f, Canvas->SizeX, 3.0f);
}

void ABreachpointHUD::DrawShadowedText(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Color)
{
	DrawText(Text, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), X + 1.5f, Y + 1.5f, Font, Scale);
	DrawText(Text, Color, X, Y, Font, Scale);
}
