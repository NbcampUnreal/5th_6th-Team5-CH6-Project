// HealItemWidget.cpp

#include "UI_KWJ/HealItem/HealItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UHealItemWidget::UpdateHealCount(int32 Current, int32 Max)
{
	if (TXT_HealCount)
	{
		FString CountStr = FString::Printf(TEXT("%d / %d"), Current, Max);
		TXT_HealCount->SetText(FText::FromString(CountStr));
	}
}

void UHealItemWidget::SetHealUIVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
