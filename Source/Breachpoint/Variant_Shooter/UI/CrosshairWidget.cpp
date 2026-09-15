// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrosshairWidget.h"

void UCrosshairWidget::AddSpread(float Amount)
{
	CurrentSpread = FMath::Clamp(CurrentSpread + Amount, MinSpread, MaxSpread);
	BP_UpdateCrosshair(CurrentSpread);
}

void UCrosshairWidget::UpdateSpread(float DeltaTime)
{
	if (CurrentSpread > MinSpread)
	{
		CurrentSpread = FMath::Max(CurrentSpread - SpreadRecoveryRate * DeltaTime, MinSpread);
		BP_UpdateCrosshair(CurrentSpread);
	}
}

void UCrosshairWidget::SetCrosshairStyle(ECrosshairStyle NewStyle)
{
	if (CrosshairStyle != NewStyle)
	{
		CrosshairStyle = NewStyle;
		BP_OnStyleChanged(NewStyle);
	}
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateSpread(InDeltaTime);
}
