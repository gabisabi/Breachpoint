// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Modes/BreachpointHUD.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FLinearColor BreachpointAccent(1.0f, 0.62f, 0.11f);
	const FLinearColor BreachpointWhite(0.95f, 0.97f, 1.0f);
	const FLinearColor BreachpointDim(0.75f, 0.8f, 0.85f);
}

void ABreachpointHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	UFont* Large = GEngine->GetLargeFont();
	UFont* Medium = GEngine->GetMediumFont();

	const float SizeX = Canvas->SizeX;
	const float SizeY = Canvas->SizeY;
	const float BigScale = FMath::Max(2.0f, SizeY / 360.0f);
	const float MidScale = FMath::Max(1.25f, SizeY / 600.0f);

	ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	// objective line at the top
	if (Mode && !Mode->GetObjectiveText().IsEmpty())
	{
		DrawCenteredText(Mode->GetObjectiveText(), Medium, MidScale, SizeY * 0.03f, BreachpointWhite);
	}

	// center banner
	if (Mode && !Mode->IsMatchEnded() && GetWorld()->GetTimeSeconds() < Mode->GetBannerUntil() && !Mode->GetBannerText().IsEmpty())
	{
		DrawCenteredText(Mode->GetBannerText(), Large, BigScale, SizeY * 0.28f, BreachpointAccent);
	}

	// end screen
	if (Mode && Mode->IsMatchEnded())
	{
		DrawPanel(SizeY * 0.3f, SizeY * 0.28f, 0.6f);
		float Y = SizeY * 0.34f;
		Y += DrawCenteredText(Mode->GetEndText(), Large, BigScale, Y, BreachpointAccent) + SizeY * 0.03f;
		DrawCenteredText(TEXT("ENTER - restart    TAB - game modes"), Medium, MidScale, Y, BreachpointWhite);
	}

	// mode select menu
	if (bMenuOpen)
	{
		DrawPanel(SizeY * 0.2f, SizeY * 0.52f, 0.7f);
		float Y = SizeY * 0.24f;
		Y += DrawCenteredText(TEXT("BREACHPOINT"), Large, BigScale, Y, BreachpointAccent) + SizeY * 0.04f;
		Y += DrawCenteredText(TEXT("1  -  Arena (free play)"), Medium, MidScale, Y, BreachpointWhite) + SizeY * 0.018f;
		Y += DrawCenteredText(TEXT("2  -  Horde (survive the waves)"), Medium, MidScale, Y, BreachpointWhite) + SizeY * 0.018f;
		Y += DrawCenteredText(TEXT("3  -  Gun Game (kill to upgrade)"), Medium, MidScale, Y, BreachpointWhite) + SizeY * 0.018f;
		Y += DrawCenteredText(TEXT("4  -  Team Deathmatch (first to 15)"), Medium, MidScale, Y, BreachpointWhite) + SizeY * 0.03f;
		DrawCenteredText(TEXT("TAB - close    ENTER - restart match"), Medium, MidScale * 0.9f, Y, BreachpointDim);
	}
	else if (GetWorld()->GetTimeSeconds() < 14.0f)
	{
		// early-match hint
		DrawCenteredText(TEXT("TAB - game mode menu"), Medium, MidScale * 0.9f, SizeY * 0.93f, BreachpointDim);
	}
}

float ABreachpointHUD::DrawCenteredText(const FString& Text, UFont* Font, float Scale, float Y, const FLinearColor& Color)
{
	float W = 0.0f, H = 0.0f;
	Canvas->TextSize(Font, Text, W, H, Scale, Scale);

	const float X = (Canvas->SizeX - W) * 0.5f;

	// soft shadow for readability
	DrawText(Text, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), X + 2.0f, Y + 2.0f, Font, Scale);
	DrawText(Text, Color, X, Y, Font, Scale);

	return H;
}

void ABreachpointHUD::DrawPanel(float Y, float Height, float Alpha)
{
	DrawRect(FLinearColor(0.02f, 0.05f, 0.09f, Alpha), 0.0f, Y, Canvas->SizeX, Height);
	DrawRect(BreachpointAccent, 0.0f, Y, Canvas->SizeX, 3.0f);
	DrawRect(BreachpointAccent, 0.0f, Y + Height - 3.0f, Canvas->SizeX, 3.0f);
}
