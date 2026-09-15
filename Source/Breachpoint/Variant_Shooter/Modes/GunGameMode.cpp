// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/Modes/GunGameMode.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Variant_Shooter/Weapons/ShooterPickup.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

AGunGameMode::AGunGameMode()
{
	ModeOptions = TEXT("?game=/Script/Breachpoint.GunGameMode");

	// pistol -> rifle -> grenade launcher
	static ConstructorHelpers::FClassFinder<AShooterWeapon> PistolFinder(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Weapons/BP_ShooterWeapon_Pistol"));
	static ConstructorHelpers::FClassFinder<AShooterWeapon> RifleFinder(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Weapons/BP_ShooterWeapon_Rifle"));
	static ConstructorHelpers::FClassFinder<AShooterWeapon> GrenadeFinder(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Weapons/BP_ShooterWeapon_GrenadeLauncher"));

	if (PistolFinder.Succeeded())
	{
		WeaponProgression.Add(PistolFinder.Class);
	}
	if (RifleFinder.Succeeded())
	{
		WeaponProgression.Add(RifleFinder.Class);
	}
	if (GrenadeFinder.Succeeded())
	{
		WeaponProgression.Add(GrenadeFinder.Class);
	}
}

void AGunGameMode::BeginPlay()
{
	Super::BeginPlay();

	ShowBanner(TEXT("GUN GAME - KILL TO UPGRADE"), 4.0f);
	UpdateObjective();

	// remove weapon pickups so the progression can't be skipped
	TArray<AActor*> Pickups;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterPickup::StaticClass(), Pickups);

	for (AActor* Pickup : Pickups)
	{
		Pickup->Destroy();
	}

	// keep the player's loadout and the enemy count in shape
	GetWorld()->GetTimerManager().SetTimer(LoadoutTimer, this, &AGunGameMode::MaintainMatch, 1.0f, true, 1.5f);
}

void AGunGameMode::OnEnemyKilled(uint8 TeamByte)
{
	++Kills;

	const int32 NewTier = Kills / KillsPerTier;

	if (NewTier >= WeaponProgression.Num())
	{
		EndMatch(FString::Printf(TEXT("GUN GAME CHAMPION - %d KILLS"), Kills));
		return;
	}

	if (NewTier != Tier)
	{
		Tier = NewTier;
		GrantTierWeapon();
		ShowBanner(TEXT("WEAPON UPGRADE"), 2.5f);
	}

	UpdateObjective();
}

void AGunGameMode::MaintainMatch()
{
	if (bMatchEnded)
	{
		return;
	}

	// re-grant the tier weapon after respawns (respawned pawns come back empty-handed)
	if (AShooterCharacter* Player = GetPlayerCharacter())
	{
		if (!Player->IsDead() && !Player->HasAnyWeapon())
		{
			GrantTierWeapon();
		}
	}

	// keep enough targets alive
	TArray<AActor*> LivingEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPC::StaticClass(), LivingEnemies);

	int32 Alive = 0;
	for (AActor* Enemy : LivingEnemies)
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

void AGunGameMode::GrantTierWeapon()
{
	if (!WeaponProgression.IsValidIndex(Tier))
	{
		return;
	}

	if (AShooterCharacter* Player = GetPlayerCharacter())
	{
		Player->RemoveAllWeapons();
		Player->AddWeaponClass(WeaponProgression[Tier]);
	}
}

void AGunGameMode::UpdateObjective()
{
	const int32 KillsIntoTier = Kills - Tier * KillsPerTier;

	SetObjective(FString::Printf(TEXT("GUN GAME - weapon %d/%d  |  %d/%d kills to next  |  %d total"),
		Tier + 1, WeaponProgression.Num(), KillsIntoTier, KillsPerTier, Kills));
}
