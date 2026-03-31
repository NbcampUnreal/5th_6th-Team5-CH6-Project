// ItemNotifySubsystem.cpp

#include "UI_KWJ/ItemNotify/ItemNotifySubsystem.h"
#include "UI_KWJ/ItemNotify/ItemNotifyWidget.h"
#include "WardGameInstanceSubsystem.h"
#include "UI_KWJ/Reading/WardDocumentDataTable.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Ward_Zero.h"

void UItemNotifySubsystem::ShowItemNotifyByIndex(int32 DocIdx)
{
	UGameInstance* GI = GetLocalPlayer()->GetGameInstance();
	if (!GI) return;

	UWardGameInstanceSubsystem* SaveGI = GI->GetSubsystem<UWardGameInstanceSubsystem>();
	if (!SaveGI) return;

	// 서류(21+)는 4번 인덱스 데이터로 통합 — 최초 1회만 알림
	const int32 DocumentStartIndex = 21;
	const int32 DocumentNotifyIndex = 4;
	int32 NotifyKey = (DocIdx >= DocumentStartIndex) ? DocumentNotifyIndex : DocIdx;
	int32 DisplayIdx = (DocIdx >= DocumentStartIndex) ? DocumentNotifyIndex : DocIdx;

	// 이미 알림을 띄운 아이템/그룹이면 무시
	if (SaveGI->IsItemNotified(NotifyKey)) return;

	// DataTable에서 정보 조회 (노티파이용 인덱스 사용)
	FWardDocumentEntry Entry;
	if (!SaveGI->GetDocumentEntry(DisplayIdx, Entry)) return;

	// 위젯 표시 — NotifyDisplayName이 있으면 우선 사용, 없으면 Title
	FText DisplayName = Entry.NotifyDisplayName.IsEmpty() ? Entry.Title : Entry.NotifyDisplayName;
	UTexture2D* Tex = Entry.ThumbnailImage.LoadSynchronous();
	UE_LOG(LogWard_Zero, Log, TEXT("노티파이 이미지: 경로=[%s] IsValid=%d 로드=%s"),
		*Entry.ThumbnailImage.ToString(),
		Entry.ThumbnailImage.IsValid() ? 1 : 0,
		Tex ? TEXT("성공") : TEXT("실패"));
	ShowItemNotify(DisplayName, Tex, Entry.KeyHint);

	// 최초 습득 기록
	SaveGI->MarkItemNotified(NotifyKey);

	UE_LOG(LogWard_Zero, Log, TEXT("아이템 알림 표시: [%d] (키=%d, 표시=%d) %s"),
		DocIdx, NotifyKey, DisplayIdx, *DisplayName.ToString());
}

void UItemNotifySubsystem::ShowItemNotify(const FText& ItemName, UTexture2D* ItemImage, const FText& KeyHint)
{
	UItemNotifyWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		// 위젯 닫힘 콜백 바인딩
		Widget->OnHiddenDelegate.BindUObject(this, &UItemNotifySubsystem::HandleNotifyHidden);
		Widget->ShowNotify(ItemName, ItemImage, KeyHint);
		bNotifyActive = true;

		// 게임 일시정지 + UI 입력 모드 (닫기 버튼 클릭 가능하도록)
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}
}

void UItemNotifySubsystem::HideItemNotify()
{
	if (NotifyWidget)
	{
		NotifyWidget->HideNotify();
	}
}

void UItemNotifySubsystem::HandleNotifyHidden()
{
	bNotifyActive = false;

	// 일시정지 해제 + 게임 입력 모드 복원
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}

	OnNotifyHidden.Broadcast();
	UE_LOG(LogWard_Zero, Log, TEXT("아이템 알림 닫힘 → 대기 위젯 실행"));
}

UItemNotifyWidget* UItemNotifySubsystem::GetOrCreateWidget()
{
	if (IsValid(NotifyWidget))
	{
		if (!NotifyWidget->IsInViewport())
		{
			NotifyWidget->AddToViewport(600);
			NotifyWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return NotifyWidget;
	}

	NotifyWidget = nullptr;

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UItemNotifyWidget>(
			nullptr,
			TEXT("/Game/UI/pickup/WBP_ItemNotify.WBP_ItemNotify_C")
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
		NotifyWidget->AddToViewport(600);
		NotifyWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return NotifyWidget;
}
