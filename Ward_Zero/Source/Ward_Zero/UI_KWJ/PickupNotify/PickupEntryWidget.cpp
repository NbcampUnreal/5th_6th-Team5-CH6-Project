// PickupEntryWidget.cpp

#include "UI_KWJ/PickupNotify/PickupEntryWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Ward_Zero.h"

void UPickupEntryWidget::InitEntry(const FText& InText, float Duration)
{
	if (TXT_PickupText)
	{
		TXT_PickupText->SetText(InText);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RemoveTimerHandle,
			this,
			&UPickupEntryWidget::RemoveSelf,
			Duration,
			false
		);
	}

	UE_LOG(LogWard_Zero, Log, TEXT("픽업 알림 추가: %s (%.1f초)"), *InText.ToString(), Duration);
}

void UPickupEntryWidget::RemoveSelf()
{
	RemoveFromParent();
}

void UPickupEntryWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RemoveTimerHandle);
	}

	Super::NativeDestruct();
}
