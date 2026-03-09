// SaveSlotItem.cpp
// 개별 세이브 슬롯 위젯 ? 텍스트·썸네일 표시 및 클릭 이벤트 전달

#include "UI_KWJ/Save/SaveSlotItem.h"
#include "UI_KWJ/Save/WardSaveGame.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

// ────────────────────────────────────────────
//  초기화
// ────────────────────────────────────────────

void USaveSlotItem::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
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

	// 5) 썸네일 ? SlotName으로 세이브 파일 로드하여 스크린샷 추출
	if (IMG_Thumbnail)
	{
		UWardSaveGame* SaveGame = Cast<UWardSaveGame>(
			UGameplayStatics::LoadGameFromSlot(InInfo.SlotName, 0));

		if (SaveGame && SaveGame->ScreenshotData.Num() > 0)
		{
			const int32 Width = SaveGame->ScreenshotWidth;
			const int32 Height = SaveGame->ScreenshotHeight;

			// BGRA 포맷 트랜지언트 텍스처 생성
			UTexture2D* Thumbnail = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
			if (Thumbnail)
			{
				if (Thumbnail->GetPlatformData() && Thumbnail->GetPlatformData()->Mips.Num() > 0)
				{
					FTexture2DMipMap& Mip = Thumbnail->GetPlatformData()->Mips[0];
					void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(Data, SaveGame->ScreenshotData.GetData(), SaveGame->ScreenshotData.Num());
					Mip.BulkData.Unlock();

					// GPU에 업로드
					Thumbnail->UpdateResource();

					// 이미지 위젯에 적용
					IMG_Thumbnail->SetBrushFromTexture(Thumbnail);
				}
			}
		}
	}
}

// ────────────────────────────────────────────
//  버튼 클릭 콜백
// ────────────────────────────────────────────

void USaveSlotItem::OnButtonClicked()
{
	// 부모(SaveWidget)에 어떤 슬롯이 선택됐는지 알림
	OnClicked_SlotItem.ExecuteIfBound(SlotInfo);
}
