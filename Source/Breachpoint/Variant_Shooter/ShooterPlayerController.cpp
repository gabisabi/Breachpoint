// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "Breachpoint.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Variant_Shooter/Modes/BreachpointHUD.h"
#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "Engine/Engine.h"
#include "Components/InputComponent.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogBreachpoint, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogBreachpoint, Error, TEXT("Could not spawn bullet counter widget."));

		}
	}

	// ── Key bindings for HUD navigation ──────────────────────────
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AShooterPlayerController::OnUpPressed);
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AShooterPlayerController::OnDownPressed);
		InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &AShooterPlayerController::OnLeftPressed);
		InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &AShooterPlayerController::OnRightPressed);
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AShooterPlayerController::OnConfirmPressed);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AShooterPlayerController::OnBackPressed);
		InputComponent->BindKey(EKeys::BackSpace, IE_Pressed, this, &AShooterPlayerController::OnBackPressed);
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AShooterPlayerController::OnTabPressed);
	}
}

// ═══════════════════════════════════════════════════════════════════════════════
// HUD Navigation — route key events to the canvas HUD
// ═══════════════════════════════════════════════════════════════════════════════

void AShooterPlayerController::OnUpPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateUp();
	}
}

void AShooterPlayerController::OnDownPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateDown();
	}
}

void AShooterPlayerController::OnLeftPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateLeft();
	}
}

void AShooterPlayerController::OnRightPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->NavigateRight();
	}
}

void AShooterPlayerController::OnConfirmPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->Confirm();
	}
}

void AShooterPlayerController::OnBackPressed()
{
	if (ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		HUD->GoBack();
	}
}

void AShooterPlayerController::OnTabPressed()
{
	ABreachpointHUD* HUD = Cast<ABreachpointHUD>(GetHUD());
	if (!HUD)
	{
		return;
	}

	// If in a lobby screen, Tab does nothing (use Escape to go back)
	if (HUD->CurrentScreen != ELobbyScreen::None)
	{
		return;
	}

	// If match has ended, Tab returns to lobby
	ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(GetWorld()->GetAuthGameMode());
	if (Mode && Mode->IsMatchEnded())
	{
		UGameplayStatics::OpenLevel(this,
			FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true,
			TEXT("?game=/Script/Breachpoint.LobbyGameMode"));
		return;
	}

	// Otherwise toggle pause overlay
	HUD->bPauseMenuOpen = !HUD->bPauseMenuOpen;
	HUD->PauseSelectedIndex = 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Pawn management (unchanged from original)
// ═══════════════════════════════════════════════════════════════════════════════

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// set the team
		ShooterCharacter->SetTeam(TeamByte);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	if (TeamByte < TeamTags.Num())
	{
		// find the player start
		TArray<AActor*> ActorList;
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), APlayerStart::StaticClass(), TeamTags[TeamByte], ActorList);

		if (ActorList.Num() > 0)
		{
			// select a random player start
			AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

			// spawn a character at the player start
			const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

			if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
			{
				// possess the character
				Possess(RespawnedCharacter);
			}
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

bool AShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AShooterPlayerController::SetTeam(uint8 Team)
{
	TeamByte = Team;

	// if we already have a pawn, set its team
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn()))
	{
		ShooterCharacter->SetTeam(Team);
	}
}
