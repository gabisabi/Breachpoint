// Copyright Breachpoint. All Rights Reserved.

#include "LobbyPlayerController.h"
#include "LobbyUI.h"
#include "LobbyGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	// Show the mouse cursor in the lobby
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		// Set input mode to UI-only in the lobby
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		CreateLobbyUI();
	}
}

void ALobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyLobbyUI();
	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerController, bIsReady);
}

void ALobbyPlayerController::ToggleReady()
{
	SetReady(!bIsReady);
}

void ALobbyPlayerController::SetReady(bool bNewReady)
{
	if (bIsReady != bNewReady)
	{
		bIsReady = bNewReady;

		// Notify the server
		if (!HasAuthority())
		{
			ServerSetReady(bNewReady);
		}

		// Update the UI
		if (LobbyUIWidget)
		{
			LobbyUIWidget->BP_OnPlayerReadyChanged(bIsReady);
		}
	}
}

void ALobbyPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;
}

void ALobbyPlayerController::CreateLobbyUI()
{
	if (LobbyUIWidget || !LobbyUIClass)
	{
		return;
	}

	LobbyUIWidget = CreateWidget<ULobbyUI>(this, LobbyUIClass);
	if (LobbyUIWidget)
	{
		LobbyUIWidget->AddToViewport();

		// Populate the UI with available maps and modes from the game mode
		if (ALobbyGameMode* LobbyGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()))
		{
			LobbyUIWidget->BP_PopulateMapList(LobbyGM->GetAvailableMaps());
			LobbyUIWidget->BP_PopulateGameModeList(LobbyGM->GetAvailableGameModes());
		}
	}
}

void ALobbyPlayerController::DestroyLobbyUI()
{
	if (LobbyUIWidget)
	{
		LobbyUIWidget->RemoveFromParent();
		LobbyUIWidget = nullptr;
	}
}
