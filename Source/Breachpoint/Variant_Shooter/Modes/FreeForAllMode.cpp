// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Modes/FreeForAllMode.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

AFreeForAllMode::AFreeForAllMode()
{
	ModeOptions = TEXT("?game=/Script/Breachpoint.FreeForAllMode");
}

// ── Lifecycle ───────────────────────────────────────────────────────────────

void AFreeForAllMode::BeginPlay()
{
	Super::BeginPlay();

	// Register the player as team 0
	PlayerTeamByte = 0;
	KillsByTeam.Add(PlayerTeamByte, 0);
	TeamNames.Add(PlayerTeamByte, TEXT("YOU"));

	// Assign team 0 to the player character
	if (AShooterCharacter* PlayerChar = GetPlayerCharacter())
	{
		PlayerChar->SetTeam(PlayerTeamByte);
	}

	ShowBanner(FString::Printf(TEXT("FREE FOR ALL - FIRST TO %d KILLS"), TargetScore), 4.0f);
	UpdateObjective();

	// Periodically top up enemies
	GetWorld()->GetTimerManager().SetTimer(TopUpTimer, this, &AFreeForAllMode::TopUpEnemies, 4.0f, true, 6.0f);
}

// ── Kill handling ───────────────────────────────────────────────────────────

void AFreeForAllMode::OnEnemyKilled(uint8 TeamByte)
{
	// The player killed an enemy — credit goes to the player
	int32& PlayerKills = KillsByTeam.FindOrAdd(PlayerTeamByte);
	++PlayerKills;

	UpdateObjective();

	if (PlayerKills >= TargetScore)
	{
		EndMatch(FString::Printf(TEXT("VICTORY - %d KILLS\n\n%s"), PlayerKills, *BuildLeaderboard()));
	}
}

void AFreeForAllMode::OnPlayerDied()
{
	// Credit a random alive enemy with the kill
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPC::StaticClass(), Enemies);

	for (AActor* Enemy : Enemies)
	{
		if (!Enemy->ActorHasTag(FName("Dead")))
		{
			AShooterCharacter* EnemyChar = Cast<AShooterCharacter>(Enemy);
			if (EnemyChar)
			{
				// Find the enemy's team or fallback to team 1
				uint8 EnemyTeam = 1;
				for (const auto& Pair : TeamNames)
				{
					// Credit the first alive enemy found
					EnemyTeam = Pair.Key;
					if (Pair.Key != PlayerTeamByte)
					{
						break;
					}
				}

				int32& Kills = KillsByTeam.FindOrAdd(EnemyTeam);
				++Kills;

				if (Kills >= TargetScore)
				{
					EndMatch(FString::Printf(TEXT("DEFEAT - %s WINS WITH %d KILLS\n\n%s"),
						*TeamNames.FindRef(EnemyTeam), Kills, *BuildLeaderboard()));
				}
				break;
			}
		}
	}

	UpdateObjective();
}

// ── Spawning ────────────────────────────────────────────────────────────────

void AFreeForAllMode::SpawnFFAEnemy()
{
	AShooterNPC* Enemy = SpawnEnemy();
	if (Enemy)
	{
		// Assign a unique team so every bot fights everyone
		const uint8 Team = NextTeamByte++;
		Enemy->SetTeam(Team);

		KillsByTeam.Add(Team, 0);
		TeamNames.Add(Team, FString::Printf(TEXT("BOT %d"), static_cast<int32>(Team)));
	}
}

void AFreeForAllMode::TopUpEnemies()
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
		SpawnFFAEnemy();
	}
}

// ── UI ──────────────────────────────────────────────────────────────────────

void AFreeForAllMode::UpdateObjective()
{
	const int32 PlayerKills = KillsByTeam.FindRef(PlayerTeamByte);
	SetObjective(FString::Printf(TEXT("FREE FOR ALL - %d / %d KILLS  |  %s"), PlayerKills, TargetScore, *BuildLeaderboard()));
}

FString AFreeForAllMode::BuildLeaderboard() const
{
	// Sort players by kills descending
	TArray<TPair<uint8, int32>> Sorted;
	for (const auto& Pair : KillsByTeam)
	{
		Sorted.Add(TPair<uint8, int32>(Pair.Key, Pair.Value));
	}

	Sorted.Sort([](const TPair<uint8, int32>& A, const TPair<uint8, int32>& B)
	{
		return A.Value > B.Value;
	});

	FString Board;
	int32 Rank = 1;
	for (const auto& Entry : Sorted)
	{
		const FString* Name = TeamNames.Find(Entry.Key);
		const FString DisplayName = Name ? *Name : FString::Printf(TEXT("Team %d"), static_cast<int32>(Entry.Key));
		Board += FString::Printf(TEXT("#%d %s: %d  "), Rank, *DisplayName, Entry.Value);
		++Rank;

		// Only show top 5 on the HUD
		if (Rank > 5)
		{
			break;
		}
	}

	return Board;
}

// ── Team assignment ─────────────────────────────────────────────────────────

uint8 AFreeForAllMode::AssignTeam(AShooterCharacter* Character)
{
	if (!Character)
	{
		return 0;
	}

	const uint8 Team = NextTeamByte++;
	Character->SetTeam(Team);
	KillsByTeam.Add(Team, 0);
	TeamNames.Add(Team, FString::Printf(TEXT("Player %d"), static_cast<int32>(Team)));

	return Team;
}
