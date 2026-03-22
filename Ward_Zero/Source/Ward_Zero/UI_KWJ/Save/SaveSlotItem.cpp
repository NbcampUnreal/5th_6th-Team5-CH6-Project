// SaveSlotItem.cpp
// 개별 세이브 슬롯 위젯 — 텍스트·썸네일 표시 및 클릭 이벤트 전달

#include "UI_KWJ/Save/SaveSlotItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

// ────────────────────────────────────────────
//  초기화
// ────────────────────────────────────────────

void USaveSlotItem::NativeConstruct()
{
	Super::NativeConstruct();


}
void USaveSlotItem::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (BTN_Slot)
	{
		BTN_Slot->OnClicked.AddDynamic(this, &USaveSlotItem::OnButtonClicked);
	}
}
// ────────────────────────────────────────────
//  슬롯 데이터 설정 (외부 호출)
// ────────────────────────────────────────────

void USaveSlotItem::SetSlotInfo(const FSaveFileInfo& InInfo)
{
	SlotInfo = InInfo;

	// 1) 표시 이름
	if (TXT_SlotLabel)
	{
		TXT_SlotLabel->SetText(FText::FromString(InInfo.DisplayName));
	}

	// 2) 날짜/시간 포맷: "2025/06/15 14:30"
	if (TXT_DateTime)
	{
		FString TimeStr = InInfo.SaveDateTime.ToString(TEXT("%Y/%m/%d %H:%M"));
		TXT_DateTime->SetText(FText::FromString(TimeStr));
	}

	// 3) 레벨(맵) 이름
	if (TXT_LevelName)
	{
		TXT_LevelName->SetText(FText::FromName(InInfo.LevelName));
	}

	// 4) 플레이 타임 (HH:MM:SS)
	if (TXT_Playtime)
	{
		int32 TotalSec = FMath::FloorToInt(InInfo.PlayTimeSeconds);
		int32 H = TotalSec / 3600;
		int32 M = (TotalSec % 3600) / 60;
		int32 S = TotalSec % 60;
		TXT_Playtime->SetText(FText::FromString(
			FString::Printf(TEXT("%02d:%02d:%02d"), H, M, S)));
	}

	// 5) 썸네일 — GetSaveFileList()에서 이미 생성된 Thumbnail 사용
	if (IMG_Thumbnail && InInfo.Thumbnail)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(InInfo.Thumbnail);
		Brush.ImageSize = FVector2D(
			InInfo.Thumbnail->GetSizeX(),
			InInfo.Thumbnail->GetSizeY()
		);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		IMG_Thumbnail->SetBrush(Brush);
		IMG_Thumbnail->SetColorAndOpacity(FLinearColor::White);
	}
}

// ────────────────────────────────────────────
//  버튼 클릭 콜백
// ────────────────────────────────────────────

void USaveSlotItem::OnButtonClicked()
{
	OnClicked_SlotItem.ExecuteIfBound(SlotInfo);
}