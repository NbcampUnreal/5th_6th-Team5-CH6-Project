// SaveWidget.cpp

#include "UI_KWJ/Save/SaveWidget.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/Save/SaveSlotItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"
#include "Engine/Texture2D.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

void USaveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_SaveNew)
	{
		BTN_SaveNew->OnClicked.AddDynamic(this, &USaveWidget::OnSaveNewClicked);
	}
	if (BTN_Close)
	{
		BTN_Close->OnClicked.AddDynamic(this, &USaveWidget::OnCloseClicked);
	}
	if (BTN_Load)
	{
		BTN_Load->OnClicked.AddDynamic(this, &USaveWidget::OnLoadClicked);
	}
	if (BTN_Delete)
	{
		BTN_Delete->OnClicked.AddDynamic(this, &USaveWidget::OnDeleteClicked);
	}
	if (BTN_Overwrite)
	{
		BTN_Overwrite->OnClicked.AddDynamic(this, &USaveWidget::OnOverwriteClicked);
	}
}

// ════════════════════════════════════════════════════════
//  세이브 목록 갱신
// ════════════════════════════════════════════════════════

void USaveWidget::RefreshSaveList()
{
	if (!ScrollBox_SaveList) return;
	ScrollBox_SaveList->ClearChildren();

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (!SaveSub) return;

	CachedSaveFiles = SaveSub->GetSaveFileList();

	for (int32 i = 0; i < CachedSaveFiles.Num(); i++)
	{
		const FSaveFileInfo& Info = CachedSaveFiles[i];

		if (!SaveSlotItemClass)
		{
			// SaveSlotItem 위젯 클래스가 없으면 기본 버튼으로 폴백
			UButton* SlotButton = NewObject<UButton>(this);
			if (!SlotButton) continue;

			UTextBlock* SlotText = NewObject<UTextBlock>(SlotButton);
			if (SlotText)
			{
				FString TimeStr = Info.SaveDateTime.ToString(TEXT("%Y/%m/%d %H:%M"));
				SlotText->SetText(FText::FromString(
					FString::Printf(TEXT("%s  |  %s  |  %s"),
						*Info.DisplayName, *TimeStr, *Info.LevelName.ToString())
				));
				SlotButton->AddChild(SlotText);
			}
			ScrollBox_SaveList->AddChild(SlotButton);
			continue;
		}

		// SaveSlotItem 위젯 생성
		APlayerController* SlotPC = GetOwningPlayer();
		USaveSlotItem* SlotItem = CreateWidget<USaveSlotItem>(SlotPC, SaveSlotItemClass);
		if (!SlotItem) continue;

		SlotItem->SetSlotInfo(Info);
		SlotItem->OnClicked_SlotItem.BindUObject(this, &USaveWidget::OnSlotSelected);

		ScrollBox_SaveList->AddChild(SlotItem);
	}

	SelectedSlotName.Empty();

	UE_LOG(LogWard_Zero, Log, TEXT("세이브 목록 갱신: %d개 파일"), CachedSaveFiles.Num());
}

// ════════════════════════════════════════════════════════
//  버튼 핸들러
// ════════════════════════════════════════════════════════

void USaveWidget::OnSaveNewClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->SaveGame(TEXT(""));
		RefreshSaveList();
		UE_LOG(LogWard_Zero, Log, TEXT("새 세이브 생성"));
	}
}

void USaveWidget::OnCloseClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->HideSaveUI();
	}
}

void USaveWidget::OnLoadClicked()
{
	if (SelectedSlotName.IsEmpty())
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("로드할 세이브를 선택하세요"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->LoadGame(SelectedSlotName);
	}
}

void USaveWidget::OnDeleteClicked()
{
	if (SelectedSlotName.IsEmpty())
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("삭제할 세이브를 선택하세요"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->DeleteSave(SelectedSlotName);
		SelectedSlotName.Empty();
		RefreshSaveList();
	}
}

void USaveWidget::OnOverwriteClicked()
{
	if (SelectedSlotName.IsEmpty())
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("덮어쓸 세이브를 선택하세요"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->SaveGame(SelectedSlotName);
		RefreshSaveList();
	}
}

// ════════════════════════════════════════════════════════
//  슬롯 선택 & 상세
// ════════════════════════════════════════════════════════

void USaveWidget::OnSlotSelected(const FSaveFileInfo& Info)
{
	SelectedSlotName = Info.SlotName;
	UpdateDetailPanel(Info);
}

void USaveWidget::UpdateDetailPanel(const FSaveFileInfo& Info)
{
	if (TXT_SaveName)
	{
		TXT_SaveName->SetText(FText::FromString(Info.DisplayName));
	}
	if (TXT_SaveTime)
	{
		TXT_SaveTime->SetText(FText::FromString(
			Info.SaveDateTime.ToString(TEXT("%Y/%m/%d %H:%M:%S"))
		));
	}
	if (TXT_LevelName)
	{
		TXT_LevelName->SetText(FText::FromString(Info.LevelName.ToString()));
	}
	if (IMG_Screenshot && Info.Thumbnail)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Info.Thumbnail);
		Brush.ImageSize = FVector2D(320.f, 180.f);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		IMG_Screenshot->SetBrush(Brush);
		IMG_Screenshot->SetColorAndOpacity(FLinearColor::White);

		UE_LOG(LogWard_Zero, Log, TEXT("스크린샷 표시: 텍스처 설정 완료 (SizeX=%d)"),
			Info.Thumbnail->GetSizeX());
	}
	else
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("스크린샷 표시 실패: IMG=%s, Thumbnail=%s"),
			IMG_Screenshot ? TEXT("있음") : TEXT("없음"),
			Info.Thumbnail ? TEXT("있음") : TEXT("없음"));
	}
}
