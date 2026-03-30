// DocumentViewerWidget.cpp

#include "UI_KWJ/Reading/DocumentViewerWidget.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UDocumentViewerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Back)
	{
		BTN_Back->OnClicked.AddDynamic(this, &UDocumentViewerWidget::OnBackClicked);
	}
	if (BTN_NextPage)
	{
		BTN_NextPage->OnClicked.AddDynamic(this, &UDocumentViewerWidget::OnNextPageClicked);
	}
	if (BTN_PrevPage)
	{
		BTN_PrevPage->OnClicked.AddDynamic(this, &UDocumentViewerWidget::OnPrevPageClicked);
	}

	// 텍스트 자동 줄바꿈 설정
	if (TXT_PageContent)
	{
		TXT_PageContent->SetAutoWrapText(true);
	}
}

void UDocumentViewerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Panel_Viewer)
	{
		Panel_Viewer->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Panel_Hint)
	{
		Panel_Hint->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ────────────────────────────────────────────
//  키보드 입력 처리
// ────────────────────────────────────────────

FReply UDocumentViewerWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Escape || Key == EKeys::E)
	{
		ULocalPlayer* LP = GetOwningLocalPlayer();
		if (LP)
		{
			UDocumentSubsystem* DocSubsystem = LP->GetSubsystem<UDocumentSubsystem>();
			if (DocSubsystem)
			{
				if (DocSubsystem->bIsClosing) return FReply::Handled(); // 닫기 중 무시
				DocSubsystem->CloseDocument();
				return FReply::Handled();
			}
		}
		CloseDocument();
		return FReply::Handled();
	}

	if (Key == EKeys::Right || Key == EKeys::D)
	{
		TurnPageNext();
		return FReply::Handled();
	}
	if (Key == EKeys::Left || Key == EKeys::A)
	{
		TurnPagePrev();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ────────────────────────────────────────────
//  서류 열기 / 닫기
// ────────────────────────────────────────────

void UDocumentViewerWidget::OpenDocument(UDocumentData* InDocument)
{
	if (!InDocument || InDocument->Pages.Num() == 0)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("OpenDocument: 유효하지 않은 서류 데이터"));
		return;
	}

	CurrentDocument = InDocument;
	CurrentPageIndex = 0;

	if (IMG_DocumentBG && CurrentDocument->BackgroundTexture.IsValid())
	{
		UTexture2D* Tex = CurrentDocument->BackgroundTexture.LoadSynchronous();
		if (Tex)
		{
			IMG_DocumentBG->SetBrushFromTexture(Tex);
		}
	}

	if (TXT_Title)
	{
		TXT_Title->SetText(CurrentDocument->DocumentTitle);
	}

	UpdatePageContent();
	UpdatePageButtons();

	SetVisibility(ESlateVisibility::Visible);
	if (Panel_Viewer)
	{
		Panel_Viewer->SetVisibility(ESlateVisibility::Visible);
	}

	HideHint();

	if (Anim_Open)
	{
		PlayAnimation(Anim_Open);
	}

	SetKeyboardFocus();

	UE_LOG(LogWard_Zero, Log, TEXT("서류 열림: %s (페이지 %d/%d)"),
		*CurrentDocument->DocumentTitle.ToString(),
		CurrentPageIndex + 1,
		CurrentDocument->Pages.Num());
}

void UDocumentViewerWidget::CloseDocument()
{
	if (Anim_Close)
	{
		PlayAnimation(Anim_Close);
		float AnimLength = Anim_Close->GetEndTime();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			SetVisibility(ESlateVisibility::Collapsed);
			if (Panel_Viewer)
			{
				Panel_Viewer->SetVisibility(ESlateVisibility::Collapsed);
			}
		}, AnimLength, false);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
		if (Panel_Viewer)
		{
			Panel_Viewer->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	CurrentDocument = nullptr;
	UE_LOG(LogWard_Zero, Log, TEXT("서류 닫힘"));
}

// ────────────────────────────────────────────
//  페이지 전환
// ────────────────────────────────────────────

void UDocumentViewerWidget::TurnPageNext()
{
	if (!CurrentDocument) return;

	if (CurrentPageIndex < CurrentDocument->Pages.Num() - 1)
	{
		CurrentPageIndex++;
		UpdatePageContent();
		UpdatePageButtons();

		if (ScrollBox_Content)
		{
			ScrollBox_Content->SetScrollOffset(0.f);
		}
	}
}

void UDocumentViewerWidget::TurnPagePrev()
{
	if (!CurrentDocument) return;

	if (CurrentPageIndex > 0)
	{
		CurrentPageIndex--;
		UpdatePageContent();
		UpdatePageButtons();

		if (ScrollBox_Content)
		{
			ScrollBox_Content->SetScrollOffset(0.f);
		}
	}
}

// ────────────────────────────────────────────
//  힌트 UI
// ────────────────────────────────────────────

void UDocumentViewerWidget::ShowHint(const FText& HintText)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (Panel_Hint)
	{
		Panel_Hint->SetVisibility(ESlateVisibility::Visible);
	}
	if (TXT_InteractionHint)
	{
		TXT_InteractionHint->SetText(HintText);
	}
}

void UDocumentViewerWidget::HideHint()
{
	if (Panel_Hint)
	{
		Panel_Hint->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!CurrentDocument)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ────────────────────────────────────────────
//  내부 갱신
// ────────────────────────────────────────────

void UDocumentViewerWidget::UpdatePageContent()
{
	if (!CurrentDocument) return;
	if (!CurrentDocument->Pages.IsValidIndex(CurrentPageIndex)) return;

	const FDocumentPageData& Page = CurrentDocument->Pages[CurrentPageIndex];

	if (TXT_PageContent)
	{
		TXT_PageContent->SetText(Page.PageText);
	}

	if (TXT_PageNumber)
	{
		FText PageNumText = FText::Format(
			NSLOCTEXT("Document", "PageNumber", "{0} / {1}"),
			FText::AsNumber(CurrentPageIndex + 1),
			FText::AsNumber(CurrentDocument->Pages.Num())
		);
		TXT_PageNumber->SetText(PageNumText);
	}
}

void UDocumentViewerWidget::UpdatePageButtons()
{
	if (!CurrentDocument) return;

	int32 TotalPages = CurrentDocument->Pages.Num();

	if (BTN_NextPage)
	{
		BTN_NextPage->SetVisibility(
			CurrentPageIndex < TotalPages - 1
				? ESlateVisibility::Visible
				: ESlateVisibility::Hidden
		);
	}

	if (BTN_PrevPage)
	{
		BTN_PrevPage->SetVisibility(
			CurrentPageIndex > 0
				? ESlateVisibility::Visible
				: ESlateVisibility::Hidden
		);
	}

	if (TotalPages <= 1)
	{
		if (BTN_NextPage) BTN_NextPage->SetVisibility(ESlateVisibility::Collapsed);
		if (BTN_PrevPage) BTN_PrevPage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ────────────────────────────────────────────
//  버튼 콜백
// ────────────────────────────────────────────

void UDocumentViewerWidget::OnBackClicked()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (LP)
	{
		UDocumentSubsystem* DocSubsystem = LP->GetSubsystem<UDocumentSubsystem>();
		if (DocSubsystem)
		{
			DocSubsystem->CloseDocument();
			return;
		}
	}
	CloseDocument();
}

void UDocumentViewerWidget::OnNextPageClicked()
{
	TurnPageNext();
}

void UDocumentViewerWidget::OnPrevPageClicked()
{
	TurnPagePrev();
}
