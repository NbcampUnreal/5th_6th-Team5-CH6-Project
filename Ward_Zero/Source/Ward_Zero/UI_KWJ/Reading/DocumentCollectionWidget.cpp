// DocumentCollectionWidget.cpp

#include "UI_KWJ/Reading/DocumentCollectionWidget.h"
#include "UI_KWJ/Reading/DocumentCollectionItem.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/PauseMenu/PauseMenuSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
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
	if (!ScrollBox_Documents) return;
	ScrollBox_Documents->ClearChildren();

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UDocumentSubsystem* DocSys = LP->GetSubsystem<UDocumentSubsystem>();
	if (!DocSys) return;

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

	const TArray<UDocumentData*>& AllDocs = DocSys->GetAllDocuments();
	const TSet<UDocumentData*>& Collected = DocSys->GetCollectedDocuments();

	for (UDocumentData* Doc : AllDocs)
	{
		if (!Doc) continue;

		UDocumentCollectionItem* Item = CreateWidget<UDocumentCollectionItem>(PC, DocumentItemClass);
		if (!Item) continue;

		bool bUnlocked = Collected.Contains(Doc);
		Item->SetDocumentInfo(Doc, bUnlocked);
		Item->OnClicked_Item.BindUObject(this, &UDocumentCollectionWidget::OnDocumentItemClicked);

		ScrollBox_Documents->AddChild(Item);
	}

	// 헤더 갱신
	if (TXT_Header)
	{
		FString HeaderStr = FString::Printf(TEXT("서류 수집 (%d / %d)"), Collected.Num(), AllDocs.Num());
		TXT_Header->SetText(FText::FromString(HeaderStr));
	}

	UE_LOG(LogWard_Zero, Log, TEXT("서류 목록 갱신: %d/%d 수집"), Collected.Num(), AllDocs.Num());
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

void UDocumentCollectionWidget::OnDocumentItemClicked(UDocumentData* Document)
{
	if (!Document) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UDocumentSubsystem* DocSys = LP->GetSubsystem<UDocumentSubsystem>();
	if (DocSys)
	{
		DocSys->OpenDocument(Document);
	}
}
