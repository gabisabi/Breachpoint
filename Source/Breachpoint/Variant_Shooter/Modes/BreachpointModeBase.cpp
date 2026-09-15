// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Modes/BreachpointModeBase.h"
#include "Variant_Shooter/Modes/BreachpointHUD.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Variant_Shooter/AI/ShooterNPCSpawner.h"
#include "Variant_Shooter/UI/ShooterUI.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

ABreachpointModeBase::ABreachpointModeBase()
{
	// use the shooter Blueprint pawn, controller and score UI so all modes play identically
	static ConstructorHelpers::FClassFinder<APawn> PawnClassFinder(TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter"));
	if (PawnClassFinder.Succeeded())
	{
		DefaultPawnClass = PawnClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PCClassFinder(TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController"));
	if (PCClassFinder.Succeeded())
	{
		PlayerControllerClass = PCClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UShooterUI> UIClassFinder(TEXT("/Game/Variant_Shooter/UI/UI_Shooter"));
	if (UIClassFinder.Succeeded())
	{
		ShooterUIClass = UIClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<AShooterNPC> NPCClassFinder(TEXT("/Game/Variant_Shooter/Blueprints/AI/BP_ShooterNPC"));
	if (NPCClassFinder.Succeeded())
	{
		EnemyClass = NPCClassFinder.Class;
	}

	HUDClass = ABreachpointHUD::StaticClass();
}

void ABreachpointModeBase::BeginPlay()
{
	Super::BeginPlay();

	GatherEnemySpawnPoints();
}

void ABreachpointModeBase::IncrementTeamScore(uint8 TeamByte)
{
	Super::IncrementTeamScore(TeamByte);

	if (bMatchEnded)
	{
		return;
	}

	// the score is incremented with the team of the character that DIED
	if (TeamByte == 0)
	{
		OnPlayerDied();
	}
	else
	{
		OnEnemyKilled(TeamByte);
	}
}

void ABreachpointModeBase::RestartMatch()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true, ModeOptions);
}

void ABreachpointModeBase::ShowBanner(const FString& Text, float Duration)
{
	BannerText = Text;
	BannerUntil = GetWorld()->GetTimeSeconds() + Duration;
}

void ABreachpointModeBase::SetObjective(const FString& Text)
{
	ObjectiveText = Text;
}

void ABreachpointModeBase::EndMatch(const FString& Text)
{
	if (bMatchEnded)
	{
		return;
	}

	bMatchEnded = true;
	EndText = Text;
}

void ABreachpointModeBase::GatherEnemySpawnPoints()
{
	EnemySpawnPoints.Reset();

	// prefer the level's NPC spawner locations
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPCSpawner::StaticClass(), Found);

	for (AActor* Actor : Found)
	{
		FTransform Transform = Actor->GetActorTransform();
		Transform.AddToTranslation(FVector(0.0f, 0.0f, 90.0f));
		EnemySpawnPoints.Add(Transform);
	}

	// fall back to PlayerStarts
	if (EnemySpawnPoints.IsEmpty())
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Found);

		for (AActor* Actor : Found)
		{
			EnemySpawnPoints.Add(Actor->GetActorTransform());
		}
	}
}

AShooterNPC* ABreachpointModeBase::SpawnEnemy(float HealthMultiplier)
{
	if (!IsValid(EnemyClass) || EnemySpawnPoints.IsEmpty())
	{
		return nullptr;
	}

	const FTransform& SpawnTransform = EnemySpawnPoints[FMath::RandRange(0, EnemySpawnPoints.Num() - 1)];

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AShooterNPC* SpawnedNPC = GetWorld()->SpawnActor<AShooterNPC>(EnemyClass, SpawnTransform, SpawnParams);

	if (SpawnedNPC)
	{
		SpawnedNPC->CurrentHP *= HealthMultiplier;
	}

	return SpawnedNPC;
}

AShooterCharacter* ABreachpointModeBase::GetPlayerCharacter() const
{
	return Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}
