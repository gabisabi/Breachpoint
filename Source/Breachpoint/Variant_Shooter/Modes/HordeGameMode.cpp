// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Modes/HordeGameMode.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "TimerManager.h"
#include "Engine/World.h"

AHordeGameMode::AHordeGameMode()
{
	ModeOptions = TEXT("?game=/Script/Breachpoint.HordeGameMode");
}

void AHordeGameMode::BeginPlay()
{
	Super::BeginPlay();

	ShowBanner(TEXT("HORDE MODE - SURVIVE"), 4.0f);
	SetObjective(TEXT("HORDE - get ready..."));

	// short breather, then wave 1
	GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &AHordeGameMode::StartNextWave, 5.0f);
}

void AHordeGameMode::OnPlayerDied()
{
	EndMatch(FString::Printf(TEXT("THE HORDE GOT YOU - WAVE %d - %d KILLS"), Wave, TotalKills));

	// stop all pending spawns
	GetWorld()->GetTimerManager().ClearTimer(WaveTimer);
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	PendingSpawns = 0;
}

void AHordeGameMode::StartNextWave()
{
	++Wave;

	PendingSpawns = FMath::Min(BaseEnemiesPerWave + (Wave - 1) * EnemiesAddedPerWave, MaxEnemiesPerWave);

	ShowBanner(FString::Printf(TEXT("WAVE %d"), Wave), 3.0f);
	UpdateObjective();

	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AHordeGameMode::SpawnWaveEnemy, SpawnInterval, true, 0.5f);
}

void AHordeGameMode::SpawnWaveEnemy()
{
	if (PendingSpawns <= 0 || bMatchEnded)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		return;
	}

	const float HealthMultiplier = 1.0f + HealthGainPerWave * (Wave - 1);

	if (AShooterNPC* Enemy = SpawnEnemy(HealthMultiplier))
	{
		Enemy->OnPawnDeath.AddDynamic(this, &AHordeGameMode::OnHordeEnemyDied);

		--PendingSpawns;
		++AliveEnemies;
		UpdateObjective();
	}
}

void AHordeGameMode::OnHordeEnemyDied()
{
	AliveEnemies = FMath::Max(0, AliveEnemies - 1);
	++TotalKills;
	UpdateObjective();

	if (bMatchEnded)
	{
		return;
	}

	// wave cleared?
	if (AliveEnemies == 0 && PendingSpawns == 0)
	{
		ShowBanner(FString::Printf(TEXT("WAVE %d CLEARED"), Wave), 3.0f);
		SetObjective(FString::Printf(TEXT("HORDE - wave %d incoming..."), Wave + 1));

		GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &AHordeGameMode::StartNextWave, IntermissionTime);
	}
}

void AHordeGameMode::UpdateObjective()
{
	SetObjective(FString::Printf(TEXT("HORDE - WAVE %d  |  %d enemies left  |  %d kills"), Wave, AliveEnemies + PendingSpawns, TotalKills));
}
