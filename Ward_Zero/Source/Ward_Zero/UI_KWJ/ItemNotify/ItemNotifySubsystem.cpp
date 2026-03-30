// ItemNotifySubsystem.cpp

#include "UI_KWJ/ItemNotify/ItemNotifySubsystem.h"
#include "UI_KWJ/ItemNotify/ItemNotifyWidget.h"
#include "WardGameInstanceSubsystem.h"
#include "UI_KWJ/Reading/WardDocumentDataTable.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Ward_Zero.h"

void UItemNotifySubsystem::ShowItemNotifyByIndex(int32 DocIdx)
{
	UGameInstance* GI = GetLocalPlayer()->GetGameInstance();
	if (!GI) return;

	UWardGameInstanceSubsystem* SaveGI = GI->GetSubsystem<UWardGameInstanceSubsystem>();
	if (!SaveGI) return;

	// 이미 알림을 띄운 아이템이면 무시
	if (SaveGI->IsItemNotified(DocIdx)) return;

	// DataTable에서 정보 조회
	FWardDocumentEntry Entry;
	if (!SaveGI->GetDocumentEntry(DocIdx, Entry)) return;

	// 위젯 표시
	UTexture2D* Tex = Entry.ThumbnailImage.IsValid()
		? Entry.ThumbnailImage.LoadSynchronous() : nullptr;
	ShowItemNotify(Entry.Title, Tex, Entry.KeyHint);

	// 최초 습득 기록 (세이브에 저장됨)
	SaveGI->MarkItemNotified(DocIdx);

	UE_LOG(LogWard_Zero, Log, TEXT("아이템 알림 표시: [%d] %s"), DocIdx, *Entry.Title.ToString());
}

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
