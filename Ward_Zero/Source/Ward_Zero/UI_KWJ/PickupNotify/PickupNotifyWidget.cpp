// PickupNotifyWidget.cpp

#include "UI_KWJ/PickupNotify/PickupNotifyWidget.h"
#include "UI_KWJ/PickupNotify/PickupEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Ward_Zero.h"

void UPickupNotifyWidget::AddEntry(const FText& PickupText, float Duration)
{
	if (!ScrollBox_Entries) return;

	// EntryClass 미지정 시 자동 로드
	if (!EntryClass)
	{
		EntryClass = LoadClass<UPickupEntryWidget>(
			nullptr,
			TEXT("/Game/UI/PickupNotify/WBP_PickupEntry.WBP_PickupEntry_C")
		);
	}

	if (!EntryClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_PickupEntry를 찾을 수 없습니다!"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	UPickupEntryWidget* Entry = CreateWidget<UPickupEntryWidget>(PC, EntryClass);
	if (!Entry) return;

	// 스크롤 박스 하단에 추가 (이전 항목은 위에 남고 새 항목이 아래에 붙음)
	ScrollBox_Entries->AddChild(Entry);

	// 항목 초기화 및 타이머 시작
	Entry->InitEntry(PickupText, Duration);

	// 새 항목이 보이도록 맨 아래로 스크롤
	ScrollBox_Entries->ScrollToEnd();
}
