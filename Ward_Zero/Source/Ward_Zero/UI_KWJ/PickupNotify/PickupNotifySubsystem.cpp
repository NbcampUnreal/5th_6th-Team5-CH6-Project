// PickupNotifySubsystem.cpp

#include "UI_KWJ/PickupNotify/PickupNotifySubsystem.h"
#include "UI_KWJ/PickupNotify/PickupNotifyWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

// ════════════════════════════════════════════════════════
//  픽업 알림 표시
// ════════════════════════════════════════════════════════

void UPickupNotifySubsystem::ShowPickup(const FText& PickupText)
{
	UPickupNotifyWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
		Widget->AddEntry(PickupText);
	}
}

// ════════════════════════════════════════════════════════
//  위젯 생성
// ════════════════════════════════════════════════════════

UPickupNotifyWidget* UPickupNotifySubsystem::GetOrCreateWidget()
{
	if (IsValid(NotifyWidget))
	{
		if (!NotifyWidget->IsInViewport())
		{
			NotifyWidget->AddToViewport(90);
			NotifyWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		return NotifyWidget;
	}

	NotifyWidget = nullptr;

	if (!NotifyWidgetClass)
	{
		NotifyWidgetClass = LoadClass<UPickupNotifyWidget>(
			nullptr,
			TEXT("/Game/UI/pickup/WBP_PickupNotify.WBP_PickupNotify_C")
		);
	}

	if (!NotifyWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_PickupNotify를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	NotifyWidget = CreateWidget<UPickupNotifyWidget>(PC, NotifyWidgetClass);
	if (NotifyWidget)
	{
		NotifyWidget->AddToViewport(90);
		NotifyWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	return NotifyWidget;
}
