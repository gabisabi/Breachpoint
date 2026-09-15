// Copyright Epic Games, Inc. All Rights Reserved.

#include "KillFeedWidget.h"

void UKillFeedWidget::AddEntry(const FKillFeedEntry& Entry)
{
	FKillFeedEntry NewEntry = Entry;

	// Stamp the current world time if not already set
	if (const UWorld* World = GetWorld())
	{
		NewEntry.Timestamp = World->GetTimeSeconds();
	}

	Entries.Add(NewEntry);

	// Trim oldest entries if we exceed the max
	while (Entries.Num() > MaxVisibleEntries)
	{
		BP_OnEntryRemoved(0);
		Entries.RemoveAt(0);
	}

	BP_OnEntryAdded(NewEntry);
}

void UKillFeedWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();

	// Remove expired entries (iterate from the back so indices stay valid for the BP event)
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (CurrentTime - Entries[i].Timestamp >= DisplayDuration)
		{
			BP_OnEntryRemoved(i);
			Entries.RemoveAt(i);
		}
	}
}
