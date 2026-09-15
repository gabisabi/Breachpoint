// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillFeedWidget.generated.h"

/**
 *  A single entry in the kill feed
 */
USTRUCT(BlueprintType)
struct BREACHPOINT_API FKillFeedEntry
{
	GENERATED_BODY()

	/** Name of the player who got the kill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed")
	FString KillerName;

	/** Name of the player who was killed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed")
	FString VictimName;

	/** Name of the weapon used for the kill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed")
	FString WeaponName;

	/** Whether this kill was a headshot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed")
	bool bIsHeadshot = false;

	/** World time when this entry was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed")
	float Timestamp = 0.0f;
};

/**
 *  Displays a scrolling feed of recent kills and deaths.
 *  Entries are automatically removed after DisplayDuration seconds.
 *  Blueprint subclass is responsible for visual layout via the BP events.
 */
UCLASS(abstract)
class BREACHPOINT_API UKillFeedWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** How long each entry stays visible before being removed (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed", meta=(ClampMin=1, ClampMax=30, Units="s"))
	float DisplayDuration = 5.0f;

	/** Maximum number of entries visible at once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KillFeed", meta=(ClampMin=1, ClampMax=20))
	int32 MaxVisibleEntries = 5;

	/** Adds a new kill feed entry. Oldest entries are removed if MaxVisibleEntries is exceeded. */
	UFUNCTION(BlueprintCallable, Category="KillFeed")
	void AddEntry(const FKillFeedEntry& Entry);

	/** Returns the current list of active entries */
	UFUNCTION(BlueprintPure, Category="KillFeed")
	const TArray<FKillFeedEntry>& GetEntries() const { return Entries; }

protected:

	/** Called when a new entry is added to the feed */
	UFUNCTION(BlueprintImplementableEvent, Category="KillFeed", meta=(DisplayName="On Entry Added"))
	void BP_OnEntryAdded(const FKillFeedEntry& Entry);

	/** Called when an entry is removed from the feed (by index) */
	UFUNCTION(BlueprintImplementableEvent, Category="KillFeed", meta=(DisplayName="On Entry Removed"))
	void BP_OnEntryRemoved(int32 Index);

	/** Tick override to check for expired entries */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

	/** Currently active entries */
	UPROPERTY()
	TArray<FKillFeedEntry> Entries;
};
