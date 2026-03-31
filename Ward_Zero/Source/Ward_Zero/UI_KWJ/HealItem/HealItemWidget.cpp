// HealItemWidget.cpp

#include "UI_KWJ/HealItem/HealItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Ward_Zero.h"

void UHealItemWidget::UpdateHealCount(int32 Current, int32 Max)
{
	if (TXT_HealCount)
	{
		FString CountStr = FString::Printf(TEXT("%d / %d"), Current, Max);
		TXT_HealCount->SetText(FText::FromString(CountStr));
	}

	// 보유 수량이 1 이상이면 표시, 0이면 숨기기
	ESlateVisibility NewVis = Current > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	SetVisibility(NewVis);

	UE_LOG(LogWard_Zero, Log, TEXT("HealItemWidget: %d/%d → %s"),
		Current, Max,
		NewVis == ESlateVisibility::HitTestInvisible ? TEXT("표시") : TEXT("숨김"));
}

void UHealItemWidget::SetHealUIVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
