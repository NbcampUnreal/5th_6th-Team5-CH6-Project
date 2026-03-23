// ItemNotifySubsystem.cpp

#include "UI_KWJ/ItemNotify/ItemNotifySubsystem.h"
#include "UI_KWJ/ItemNotify/ItemNotifyWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UItemNotifySubsystem::ShowItemNotify(const FText& ItemName, UTexture2D* ItemImage, const FText& KeyHint)
{
	UItemNotifyWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->ShowNotify(ItemName, ItemImage, KeyHint);
	}
}

void UItemNotifySubsystem::HideItemNotify()
{
	if (NotifyWidget)
	{
		NotifyWidget->HideNotify();
	}
}

UItemNotifyWidget* UItemNotifySubsystem::GetOrCreateWidget()
{
	if (IsValid(NotifyWidget))
	{
		if (!NotifyWidget->IsInViewport())
		{
			NotifyWidget->AddToViewport(80);
			NotifyWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return NotifyWidget;
	}

	NotifyWidget = nullptr;

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UItemNotifyWidget>(
			nullptr,
			TEXT("/Game/UI/ItemNotify/WBP_ItemNotify.WBP_ItemNotify_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_ItemNotify를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	NotifyWidget = CreateWidget<UItemNotifyWidget>(PC, WidgetClass);
	if (NotifyWidget)
	{
		NotifyWidget->AddToViewport(80);
		NotifyWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return NotifyWidget;
}
