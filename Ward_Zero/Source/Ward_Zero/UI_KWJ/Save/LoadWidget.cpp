// LoadWidget.cpp

#include "UI_KWJ/Save/LoadWidget.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/Save/SaveSlotItem.h"
#include "UI_KWJ/PauseMenu/PauseMenuSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"
#include "Engine/Texture2D.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

void ULoadWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Close)  BTN_Close->OnClicked.AddDynamic(this, &ULoadWidget::OnCloseClicked);
	if (BTN_Load)   BTN_Load->OnClicked.AddDynamic(this, &ULoadWidget::OnLoadClicked);
	if (BTN_Delete) BTN_Delete->OnClicked.AddDynamic(this, &ULoadWidget::OnDeleteClicked);
}

void ULoadWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

// ════════════════════════════════════════════════════════
//  세이브 목록 갱신
// ════════════════════════════════════════════════════════

void ULoadWidget::RefreshSaveList()
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
			// WBP에서 미할당 시 코드에서 직접 로드
			SaveSlotItemClass = LoadClass<UUserWidget>(
				nullptr,
				TEXT("/Game/UI/save/WBP_SaveSlot.WBP_SaveSlot_C")
			);
		}

		if (!SaveSlotItemClass)
		{
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

		APlayerController* SlotPC = GetOwningPlayer();
		USaveSlotItem* SlotItem = CreateWidget<USaveSlotItem>(SlotPC, SaveSlotItemClass);
		if (!SlotItem) continue;

		SlotItem->SetSlotInfo(Info);
		SlotItem->OnClicked_SlotItem.BindUObject(this, &ULoadWidget::OnSlotSelected);

		ScrollBox_SaveList->AddChild(SlotItem);
	}

	SelectedSlotName.Empty();
	UE_LOG(LogWard_Zero, Log, TEXT("불러오기 목록 갱신: %d개 파일"), CachedSaveFiles.Num());
}

// ════════════════════════════════════════════════════════
//  버튼 핸들러
// ════════════════════════════════════════════════════════

void ULoadWidget::OnCloseClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	// Load UI 숨기기
	USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->HideLoadUI();
	}

	// 메인메뉴에서 열렸으면 PauseMenu 안 열기
	if (!bOpenedFromMainMenu)
	{
		UPauseMenuSubsystem* PauseSys = LP->GetSubsystem<UPauseMenuSubsystem>();
		if (PauseSys)
		{
			PauseSys->ShowPauseMenu();
		}
	}
}

void ULoadWidget::OnLoadClicked()
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

void ULoadWidget::OnDeleteClicked()
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

// ════════════════════════════════════════════════════════
//  슬롯 선택 & 상세
// ════════════════════════════════════════════════════════

void ULoadWidget::OnSlotSelected(const FSaveFileInfo& Info)
{
	SelectedSlotName = Info.SlotName;
	UpdateDetailPanel(Info);
}

void ULoadWidget::UpdateDetailPanel(const FSaveFileInfo& Info)
{
	if (TXT_SaveName)
		TXT_SaveName->SetText(FText::FromString(Info.DisplayName));

	if (TXT_SaveTime)
		TXT_SaveTime->SetText(FText::FromString(Info.SaveDateTime.ToString(TEXT("%H:%M:%S"))));

	if (TXT_SaveDate)
		TXT_SaveDate->SetText(FText::FromString(Info.SaveDateTime.ToString(TEXT("%Y/%m/%d"))));

	if (TXT_LevelName)
		TXT_LevelName->SetText(FText::FromString(Info.LevelName.ToString()));

	if (TXT_Playtime)
	{
		int32 TotalSec = FMath::FloorToInt(Info.PlayTimeSeconds);
		int32 Hours = TotalSec / 3600;
		int32 Minutes = (TotalSec % 3600) / 60;
		int32 Seconds = TotalSec % 60;
		TXT_Playtime->SetText(FText::FromString(
			FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds)));
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
	}
}

void ULoadWidget::SetCloseButtonVisible(bool bVisible)
{
	if (BTN_Close)
	{
		BTN_Close->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

FReply ULoadWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		// 나가기 버튼이 보이면 (게임오버가 아니면) ESC로 닫기
		if (BTN_Close && BTN_Close->IsVisible())
		{
			OnCloseClicked();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
