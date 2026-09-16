// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorUI.h"
#include "HorrorCharacter.h"

void UHorrorUI::SetupCharacter(AHorrorCharacter* HorrorCharacter)
{
	// Unbind from the previous character if any
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnSprintMeterUpdated.RemoveDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
		BoundCharacter->OnSprintStateChanged.RemoveDynamic(this, &UHorrorUI::OnSprintStateChanged);
	}

	BoundCharacter = HorrorCharacter;

	HorrorCharacter->OnSprintMeterUpdated.AddDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
	HorrorCharacter->OnSprintStateChanged.AddDynamic(this, &UHorrorUI::OnSprintStateChanged);
}

void UHorrorUI::OnSprintMeterUpdated(float Percent)
{
	// call the BP handler
	BP_SprintMeterUpdated(Percent);
}

void UHorrorUI::OnSprintStateChanged(bool bSprinting)
{
	// call the BP handler
	BP_SprintStateChanged(bSprinting);
}
