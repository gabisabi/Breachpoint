// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Modes/TeamDeathmatchMode.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATeamDeathmatchMode::ATeamDeathmatchMode()
{
	ModeOptions = TEXT("?game=/Script/Breachpoint.TeamDeathmatchMode");
}

void ATeamDeathmatchMode::BeginPlay()
{
	Super::BeginPlay();

	ShowBanner(FString::Printf(TEXT("TEAM DEATHMATCH - FIRST TO %d"), TargetScore), 4.0f);
	UpdateObjective();

	GetWorld()->GetTimerManager().SetTimer(TopUpTimer, this, &ATeamDeathmatchMode::TopUpEnemies, 4.0f, true, 6.0f);
}

void ATeamDeathmatchMode::OnEnemyKilled(uint8 TeamByte)
{
	++PlayerScore;
	UpdateObjective();

	if (PlayerScore >= TargetScore)
	{
		EndMatch(FString::Printf(TEXT("VICTORY - %d : %d"), PlayerScore, EnemyScore));
	}
}

void ATeamDeathmatchMode::OnPlayerDied()
{
	++EnemyScore;
	UpdateObjective();

	if (EnemyScore >= TargetScore)
	{
		EndMatch(FString::Printf(TEXT("DEFEAT - %d : %d"), PlayerScore, EnemyScore));
	}
}

void ATeamDeathmatchMode::TopUpEnemies()
{
	if (bMatchEnded)
	{
		return;
	}

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPC::StaticClass(), Enemies);

	int32 Alive = 0;
	for (AActor* Enemy : Enemies)
	{
		if (!Enemy->ActorHasTag(FName("Dead")))
		{
			++Alive;
		}
	}

	if (Alive < MinEnemies)
	{
		SpawnEnemy();
	}
}

void ATeamDeathmatchMode::UpdateObjective()
{
	SetObjective(FString::Printf(TEXT("TEAM DEATHMATCH - YOU %d : %d ENEMY  |  first to %d"), PlayerScore, EnemyScore, TargetScore));
}
