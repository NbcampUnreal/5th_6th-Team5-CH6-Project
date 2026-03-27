// HealItemSubsystem.cpp

#include "UI_KWJ/HealItem/HealItemSubsystem.h"
#include "UI_KWJ/HealItem/HealItemWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UHealItemSubsystem::UpdateHealCount(int32 Current, int32 Max)
{
	UHealItemWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->UpdateHealCount(Current, Max);
	}
}

void UHealItemSubsystem::SetHealUIVisible(bool bVisible)
{
	UHealItemWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetHealUIVisible(bVisible);
	}
}

UHealItemWidget* UHealItemSubsystem::GetOrCreateWidget()
{
	if (IsValid(HealWidget))
	{
		if (!HealWidget->IsInViewport())
		{
			HealWidget->AddToViewport(60);
			HealWidget->SetAnchorsInViewport(FAnchors(1.0f, 1.0f));
			HealWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		return HealWidget;
	}

	HealWidget = nullptr;

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UHealItemWidget>(
			nullptr,
			TEXT("/Game/UI/HealItem/WBP_HealItem.WBP_HealItem_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_HealItem를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	HealWidget = CreateWidget<UHealItemWidget>(PC, WidgetClass);
	if (HealWidget)
	{
		HealWidget->AddToViewport(60);
		HealWidget->SetAnchorsInViewport(FAnchors(1.0f, 1.0f));
		HealWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	return HealWidget;
}
