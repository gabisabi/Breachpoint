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

	// Breachpoint game mode menu bindings
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AShooterPlayerController::ToggleModeMenu);
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AShooterPlayerController::HandleRestartKey);

		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AShooterPlayerController::SelectMode1);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AShooterPlayerController::SelectMode2);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AShooterPlayerController::SelectMode3);
		InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AShooterPlayerController::SelectMode4);
	}
}

void AShooterPlayerController::ToggleModeMenu()
{
	if (ABreachpointHUD* BreachpointHUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		BreachpointHUD->bMenuOpen = !BreachpointHUD->bMenuOpen;
		return;
	}

	// fallback for game modes without the Breachpoint HUD (e.g. the default arena):
	// show the menu as on-screen messages; the number keys work the same way
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(101, 8.0f, FColor::Orange, TEXT("BREACHPOINT - choose a game mode:"));
		GEngine->AddOnScreenDebugMessage(102, 8.0f, FColor::White, TEXT("  1 - Arena    2 - Horde    3 - Gun Game    4 - Team Deathmatch"));
	}
}

bool AShooterPlayerController::IsModeMenuOpen() const
{
	if (const ABreachpointHUD* BreachpointHUD = Cast<ABreachpointHUD>(GetHUD()))
	{
		return BreachpointHUD->bMenuOpen;
	}

	// without the Breachpoint HUD the fallback menu is always considered open
	return true;
}

void AShooterPlayerController::SelectGameMode(int32 ModeIndex)
{
	if (!IsModeMenuOpen())
	{
		return;
	}

	static const TCHAR* ModeOptions[] = {
		TEXT(""),
		TEXT("?game=/Script/Breachpoint.HordeGameMode"),
		TEXT("?game=/Script/Breachpoint.GunGameMode"),
		TEXT("?game=/Script/Breachpoint.TeamDeathmatchMode")
	};

	if (ModeIndex < 0 || ModeIndex > 3)
	{
		return;
	}

	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true, ModeOptions[ModeIndex]);
}

void AShooterPlayerController::HandleRestartKey()
{
	ABreachpointModeBase* Mode = Cast<ABreachpointModeBase>(GetWorld()->GetAuthGameMode());

	// restart when the match has ended, or from the menu
	if ((Mode && Mode->IsMatchEnded()) || IsModeMenuOpen())
	{
		if (Mode)
		{
			Mode->RestartMatch();
		}
		else
		{
			UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true);
		}
	}
}

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
