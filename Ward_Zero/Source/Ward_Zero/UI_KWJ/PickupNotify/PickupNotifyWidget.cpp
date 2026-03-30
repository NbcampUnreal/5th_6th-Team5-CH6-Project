// PickupNotifyWidget.cpp

#include "UI_KWJ/PickupNotify/PickupNotifyWidget.h"
#include "UI_KWJ/PickupNotify/PickupEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Ward_Zero.h"

void UPickupNotifyWidget::AddEntry(const FText& PickupText, float Duration)
{
	if (!ScrollBox_Entries) return;

	// EntryClass 미지정 시 자동 로드
	if (!EntryClass)
	{
		EntryClass = LoadClass<UPickupEntryWidget>(
			nullptr,
			TEXT("/Game/UI/pickup/WBP_PickupEntry.WBP_PickupEntry_C")
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

	// VerticalBox 하단에 추가
	ScrollBox_Entries->AddChild(Entry);

	// 항목 초기화 및 타이머 시작
	Entry->InitEntry(PickupText, Duration);
}
