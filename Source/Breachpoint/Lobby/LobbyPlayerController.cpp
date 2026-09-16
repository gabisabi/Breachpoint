// Copyright Breachpoint. All Rights Reserved.

#include "Lobby/LobbyPlayerController.h"
#include "Lobby/LobbyUI.h"
#include "Lobby/LobbyGameMode.h"
#include "Variant_Shooter/Modes/BreachpointHUD.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"

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
		// Set the canvas HUD to lobby main menu
		if (ABreachpointHUD* BreachpointHUD = Cast<ABreachpointHUD>(GetHUD()))
		{
			BreachpointHUD->CurrentScreen = ELobbyScreen::MainMenu;
			BreachpointHUD->SelectedIndex = 0;
		}

		// Also create the UMG lobby UI if a class is configured (for Blueprint-based lobbies)
		CreateLobbyUI();
	}
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind navigation keys for the canvas-drawn lobby HUD
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &ALobbyPlayerController::OnUpPressed);
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &ALobbyPlayerController::OnDownPressed);
		InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &ALobbyPlayerController::OnLeftPressed);
		InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &ALobbyPlayerController::OnRightPressed);
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ALobbyPlayerController::OnConfirmPressed);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ALobbyPlayerController::OnBackPressed);
		InputComponent->BindKey(EKeys::BackSpace, IE_Pressed, this, &ALobbyPlayerController::OnBackPressed);
	}
}

void ALobbyPlayerController::OnUpPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateUp();
	}
}

void ALobbyPlayerController::OnDownPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateDown();
	}
}

void ALobbyPlayerController::OnLeftPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateLeft();
	}
}

void ALobbyPlayerController::OnRightPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateRight();
	}
}

void ALobbyPlayerController::OnConfirmPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->Confirm();
	}
}

void ALobbyPlayerController::OnBackPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->GoBack();
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
