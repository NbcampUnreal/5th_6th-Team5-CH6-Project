// DocumentCollectionWidget.cpp

#include "UI_KWJ/Reading/DocumentCollectionWidget.h"
#include "UI_KWJ/Reading/DocumentCollectionItem.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/WardDocumentDataTable.h"
#include "WardGameInstanceSubsystem.h"
#include "UI_KWJ/PauseMenu/PauseMenuSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

void UDocumentCollectionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Close)
	{
		BTN_Close->OnClicked.AddDynamic(this, &UDocumentCollectionWidget::OnCloseClicked);
	}
}

FReply UDocumentCollectionWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnCloseClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ════════════════════════════════════════════════════════
//  서류 목록 갱신
// ════════════════════════════════════════════════════════

void UDocumentCollectionWidget::RefreshDocumentList()
{
	// WrapBox 우선, 없으면 ScrollBox
	UPanelWidget* TargetPanel = nullptr;
	if (WrapBox_Documents)
	{
		WrapBox_Documents->ClearChildren();
		TargetPanel = WrapBox_Documents;
	}
	else if (ScrollBox_Documents)
	{
		ScrollBox_Documents->ClearChildren();
		TargetPanel = ScrollBox_Documents;
	}

	if (!TargetPanel) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UGameInstance* GI = LP->GetGameInstance();
	if (!GI) return;

	UWardGameInstanceSubsystem* SaveGI = GI->GetSubsystem<UWardGameInstanceSubsystem>();
	if (!SaveGI || !SaveGI->DocumentDataTable) return;

	// 아이템 위젯 클래스 자동 로드
	if (!DocumentItemClass)
	{
		DocumentItemClass = LoadClass<UUserWidget>(
			nullptr,
			TEXT("/Game/UI/read/WBP_DocumentCollectionItem.WBP_DocumentCollectionItem_C")
		);
	}

	if (!DocumentItemClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_DocumentCollectionItem을 찾을 수 없습니다!"));
		return;
	}

	const TArray<FWardDocumentEntry>& AllEntries = SaveGI->DocumentDataTable->Entries;
	const TSet<int32>& ActiveIndices = SaveGI->GetActiveDocumentIndices();

	int32 TotalDocs = 0;
	int32 CollectedDocs = 0;

	for (const FWardDocumentEntry& Entry : AllEntries)
	{
		if (Entry.DocIndex < 20) continue;
		TotalDocs++;

		bool bUnlocked = ActiveIndices.Contains(Entry.DocIndex);
		if (bUnlocked) CollectedDocs++;

		UDocumentCollectionItem* Item = CreateWidget<UDocumentCollectionItem>(PC, DocumentItemClass);
		if (!Item) continue;

		Item->SetDocumentInfoFromEntry(Entry, bUnlocked);
		Item->OnClicked_Index.BindUObject(this, &UDocumentCollectionWidget::OnDocumentIndexClicked);

		// WrapBox면 슬롯 크기 지정
		if (WrapBox_Documents)
		{
			UWrapBoxSlot* wrapSlot = Cast<UWrapBoxSlot>(WrapBox_Documents->AddChildToWrapBox(Item));
			if (wrapSlot)
			{
				wrapSlot->SetFillEmptySpace(false);
				wrapSlot->SetPadding(FMargin(8.f));
			}
		}
		else
		{
			TargetPanel->AddChild(Item);
		}
	}

	if (TXT_Header)
	{
		FString HeaderStr = FString::Printf(TEXT("서류 수집 (%d / %d)"), CollectedDocs, TotalDocs);
		TXT_Header->SetText(FText::FromString(HeaderStr));
	}

	UE_LOG(LogWard_Zero, Log, TEXT("서류 목록 갱신: %d/%d 수집"), CollectedDocs, TotalDocs);
}

// ════════════════════════════════════════════════════════
//  버튼 핸들러
// ════════════════════════════════════════════════════════

void UDocumentCollectionWidget::OnCloseClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);

	// PauseMenu 다시 표시
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UPauseMenuSubsystem* PauseSys = LP->GetSubsystem<UPauseMenuSubsystem>();
	if (PauseSys)
	{
		PauseSys->ShowPauseMenu();
	}
}

void UDocumentCollectionWidget::OnDocumentIndexClicked(int32 DocIndex)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UDocumentSubsystem* DocSys = LP->GetSubsystem<UDocumentSubsystem>();
	if (DocSys)
	{
		DocSys->OpenDocumentByIndex(DocIndex);
	}
}
