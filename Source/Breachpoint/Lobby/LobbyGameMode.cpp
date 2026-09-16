// Copyright Breachpoint. All Rights Reserved.

#include "Lobby/LobbyGameMode.h"
#include "Lobby/LobbyPlayerController.h"
#include "Variant_Shooter/Modes/BreachpointHUD.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

// Include existing game mode classes for default population
#include "Variant_Shooter/Modes/FreeForAllMode.h"
#include "Variant_Shooter/Modes/TeamDeathmatchMode.h"
#include "Variant_Shooter/Modes/GunGameMode.h"
#include "Variant_Shooter/Modes/HordeGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
	// Default player controller for the lobby
	PlayerControllerClass = ALobbyPlayerController::StaticClass();

	// No default pawn needed in the lobby
	DefaultPawnClass = nullptr;

	// Use the canvas-drawn HUD — works immediately without Blueprint setup
	HUDClass = ABreachpointHUD::StaticClass();
}

void ALobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Populate available content if not already configured via Blueprint defaults
	if (AvailableMaps.Num() == 0)
	{
		PopulateDefaultMaps();
	}

	if (AvailableGameModes.Num() == 0)
	{
		PopulateDefaultGameModes();
	}
}

void ALobbyGameMode::StartMatch(const FString& MapPath, TSubclassOf<AGameModeBase> GameModeClass)
{
	if (!HasAuthority())
	{
		return;
	}

	if (MapPath.IsEmpty())
	{
		UE_LOG(LogGameMode, Warning, TEXT("ALobbyGameMode::StartMatch - MapPath is empty, aborting travel."));
		return;
	}

	// Build the travel URL: /Game/Maps/SomeMap?game=/Script/Breachpoint.SomeGameMode
	FString TravelURL = MapPath;

	if (GameModeClass)
	{
		const FString GameModeClassPath = GameModeClass->GetPathName();
		TravelURL += FString::Printf(TEXT("?game=%s"), *GameModeClassPath);
	}

	UE_LOG(LogGameMode, Log, TEXT("ALobbyGameMode::StartMatch - ServerTravel to: %s"), *TravelURL);
	GetWorld()->ServerTravel(TravelURL);
}

void ALobbyGameMode::PopulateDefaultMaps()
{
	// Default maps - paths should match your project's content structure.
	// Thumbnails are left null; assign them in a Blueprint subclass or via data assets.
	AvailableMaps.Add(FMapInfo(TEXT("Arena"), TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/Arena"))), 16));
	AvailableMaps.Add(FMapInfo(TEXT("Warehouse"), TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/Warehouse"))), 12));
	AvailableMaps.Add(FMapInfo(TEXT("Compound"), TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/Compound"))), 20));
}

void ALobbyGameMode::PopulateDefaultGameModes()
{
	AvailableGameModes.Add(FGameModeInfo(
		TEXT("Free For All"),
		AFreeForAllMode::StaticClass(),
		TEXT("Every operator for themselves. Last one standing wins.")
	));

	AvailableGameModes.Add(FGameModeInfo(
		TEXT("Team Deathmatch"),
		ATeamDeathmatchMode::StaticClass(),
		TEXT("Two teams clash head-to-head. Reach the score limit to win.")
	));

	AvailableGameModes.Add(FGameModeInfo(
		TEXT("Gun Game"),
		AGunGameMode::StaticClass(),
		TEXT("Score kills to cycle through an escalating weapon ladder.")
	));

	AvailableGameModes.Add(FGameModeInfo(
		TEXT("Horde"),
		AHordeGameMode::StaticClass(),
		TEXT("Cooperate against waves of increasingly deadly enemies.")
	));
}
