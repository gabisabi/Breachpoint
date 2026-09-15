// Copyright Breachpoint. All Rights Reserved.

#include "LobbyUI.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	// Default selection to first entries
	SelectedMapIndex = 0;
	SelectedGameModeIndex = 0;
}

void ULobbyUI::SelectMap(int32 MapIndex)
{
	const ALobbyGameMode* GM = GetLobbyGameMode();
	if (GM && GM->AvailableMaps.IsValidIndex(MapIndex))
	{
		SelectedMapIndex = MapIndex;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ULobbyUI::SelectMap - Invalid map index: %d"), MapIndex);
	}
}

void ULobbyUI::SelectGameMode(int32 ModeIndex)
{
	const ALobbyGameMode* GM = GetLobbyGameMode();
	if (GM && GM->AvailableGameModes.IsValidIndex(ModeIndex))
	{
		SelectedGameModeIndex = ModeIndex;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ULobbyUI::SelectGameMode - Invalid game mode index: %d"), ModeIndex);
	}
}

void ULobbyUI::ToggleReady()
{
	if (ALobbyPlayerController* PC = GetLobbyPC())
	{
		PC->ToggleReady();
	}
}

void ULobbyUI::RequestStartMatch()
{
	ALobbyGameMode* GM = GetLobbyGameMode();
	if (!GM)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULobbyUI::RequestStartMatch - No lobby game mode found (not authority?)"));
		return;
	}

	if (!GM->AvailableMaps.IsValidIndex(SelectedMapIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("ULobbyUI::RequestStartMatch - Invalid map selection: %d"), SelectedMapIndex);
		return;
	}

	if (!GM->AvailableGameModes.IsValidIndex(SelectedGameModeIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("ULobbyUI::RequestStartMatch - Invalid game mode selection: %d"), SelectedGameModeIndex);
		return;
	}

	const FMapInfo& SelectedMap = GM->AvailableMaps[SelectedMapIndex];
	const FGameModeInfo& SelectedMode = GM->AvailableGameModes[SelectedGameModeIndex];

	// Resolve the soft object path to a travel-friendly map path
	const FString MapPath = SelectedMap.MapAsset.GetLongPackageName();

	BP_OnMatchStarting();

	GM->StartMatch(MapPath, SelectedMode.ModeClass);
}

ALobbyPlayerController* ULobbyUI::GetLobbyPC() const
{
	return Cast<ALobbyPlayerController>(GetOwningPlayer());
}

ALobbyGameMode* ULobbyUI::GetLobbyGameMode() const
{
	return Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
}
