// DocumentSubsystem.cpp

#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/DocumentViewerWidget.h"
#include "UI_KWJ/Reading/DocumentCollectionWidget.h"
#include "WardGameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UDocumentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogWard_Zero, Log, TEXT("DocumentSubsystem 초기화 완료"));
}

void UDocumentSubsystem::Deinitialize()
{
	if (ViewerWidget)
	{
		ViewerWidget->RemoveFromParent();
		ViewerWidget = nullptr;
	}
	Super::Deinitialize();
}

// ────────────────────────────────────────────
//  열기 / 닫기
// ────────────────────────────────────────────

void UDocumentSubsystem::OpenDocument(UDocumentData* InDocument)
{
	if (!InDocument)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("OpenDocument: DocumentData가 null입니다."));
		return;
	}

	UDocumentViewerWidget* Viewer = GetOrCreateViewer();
	if (!Viewer)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("OpenDocument: ViewerWidget 생성 실패"));
		return;
	}

	Viewer->OpenDocument(InDocument);

	// 게임 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(Viewer->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UDocumentSubsystem::CloseDocument()
{
	if (ViewerWidget)
	{
		ViewerWidget->CloseDocument();
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		if (bOpenedFromCollection)
		{
			// 수집 UI에서 열었으면 커서 유지
			FInputModeUIOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
		else
		{
			// 인게임에서 열었으면 게임 재개
			UGameplayStatics::SetGamePaused(GetWorld(), false);
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(false);
		}
	}

	bOpenedFromCollection = false;
}

void UDocumentSubsystem::OpenDocumentByIndex(int32 DocIndex)
{
	// GameInstanceSubsystem에서 DataTable 조회
	UGameInstance* GI = GetLocalPlayer()->GetGameInstance();
	if (!GI) return;

	UWardGameInstanceSubsystem* SaveGI = GI->GetSubsystem<UWardGameInstanceSubsystem>();
	if (!SaveGI) return;

	FWardDocumentEntry Entry;
	if (!SaveGI->GetDocumentEntry(DocIndex, Entry))
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("OpenDocumentByIndex: 인덱스 %d를 찾을 수 없습니다"), DocIndex);
		return;
	}

	// DocumentData를 동적 생성해서 뷰어에 전달
	UDocumentData* TempDoc = NewObject<UDocumentData>();
	TempDoc->DocumentTitle = Entry.Title;

	// 페이지 변환 (FText → FDocumentPageData)
	for (const FText& PageText : Entry.Pages)
	{
		FDocumentPageData Page;
		Page.PageText = PageText;
		TempDoc->Pages.Add(Page);
	}

	// 배경 이미지
	if (Entry.BackgroundImage.IsValid())
	{
		TempDoc->BackgroundTexture = Entry.BackgroundImage;
	}

	bOpenedFromCollection = false;
	OpenDocument(TempDoc);

	UE_LOG(LogWard_Zero, Log, TEXT("서류 열기 (인덱스 %d): %s"), DocIndex, *Entry.Title.ToString());
}

bool UDocumentSubsystem::IsDocumentOpen() const
{
	return ViewerWidget && ViewerWidget->IsVisible();
}

// ────────────────────────────────────────────
//  페이지 제어
// ────────────────────────────────────────────

void UDocumentSubsystem::TurnPageNext()
{
	if (ViewerWidget) ViewerWidget->TurnPageNext();
}

void UDocumentSubsystem::TurnPagePrev()
{
	if (ViewerWidget) ViewerWidget->TurnPagePrev();
}

// ────────────────────────────────────────────
//  상호작용 힌트
// ────────────────────────────────────────────

void UDocumentSubsystem::ShowInteractionHint(const FText& HintText)
{
	UDocumentViewerWidget* Viewer = GetOrCreateViewer();
	if (Viewer) Viewer->ShowHint(HintText);
}

void UDocumentSubsystem::HideInteractionHint()
{
	if (ViewerWidget) ViewerWidget->HideHint();
}

// ────────────────────────────────────────────
//  위젯 생성
// ────────────────────────────────────────────

UDocumentViewerWidget* UDocumentSubsystem::GetOrCreateViewer()
{
	if (ViewerWidget) return ViewerWidget;

	if (!ViewerWidgetClass)
	{
		ViewerWidgetClass = LoadClass<UDocumentViewerWidget>(
			nullptr,
			TEXT("/Game/UI/read/WBP_DocumentViewer.WBP_DocumentViewer_C")
		);
	}

	if (!ViewerWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_DocumentViewer를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	ViewerWidget = CreateWidget<UDocumentViewerWidget>(PC, ViewerWidgetClass);
	if (ViewerWidget)
	{
		ViewerWidget->AddToViewport(500);
		ViewerWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return ViewerWidget;
}

// ════════════════════════════════════════════════════════
//  서류 수집 관리
// ════════════════════════════════════════════════════════

void UDocumentSubsystem::CollectDocument(UDocumentData* InDocument)
{
	if (!InDocument) return;

	if (!CollectedDocuments.Contains(InDocument))
	{
		CollectedDocuments.Add(InDocument);
		UE_LOG(LogWard_Zero, Log, TEXT("서류 수집: %s (%d/%d)"),
			*InDocument->DocumentTitle.ToString(),
			CollectedDocuments.Num(), AllDocuments.Num());
	}
}

bool UDocumentSubsystem::IsDocumentCollected(UDocumentData* InDocument) const
{
	return InDocument && CollectedDocuments.Contains(InDocument);
}

// ════════════════════════════════════════════════════════
//  컬렉션 UI
// ════════════════════════════════════════════════════════

void UDocumentSubsystem::ShowCollection()
{
	UDocumentCollectionWidget* Widget = GetOrCreateCollection();
	if (Widget)
	{
		Widget->RefreshDocumentList();
		Widget->SetVisibility(ESlateVisibility::Visible);
		Widget->SetKeyboardFocus();

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

void UDocumentSubsystem::HideCollection()
{
	if (CollectionWidget)
	{
		CollectionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UDocumentSubsystem::IsCollectionOpen() const
{
	return CollectionWidget && CollectionWidget->IsVisible();
}

UDocumentCollectionWidget* UDocumentSubsystem::GetOrCreateCollection()
{
	if (IsValid(CollectionWidget) && CollectionWidget->IsInViewport())
	{
		return CollectionWidget;
	}

	CollectionWidget = nullptr;

	if (!CollectionWidgetClass)
	{
		CollectionWidgetClass = LoadClass<UDocumentCollectionWidget>(
			nullptr,
			TEXT("/Game/UI/read/WBP_DocumentCollection.WBP_DocumentCollection_C")
		);
	}

	if (!CollectionWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_DocumentCollection을 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	CollectionWidget = CreateWidget<UDocumentCollectionWidget>(PC, CollectionWidgetClass);
	if (CollectionWidget)
	{
		CollectionWidget->AddToViewport(420);
		CollectionWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		CollectionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return CollectionWidget;
}
