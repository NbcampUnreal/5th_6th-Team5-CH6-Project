// DocumentSubsystem.cpp

#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/DocumentViewerWidget.h"
#include "UI_KWJ/Reading/DocumentCollectionWidget.h"
#include "UI_KWJ/ItemNotify/ItemNotifySubsystem.h"
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

	if (bIsClosing) return; // 닫기 애니메이션 중 재열기 방지

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
	if (bIsClosing) return; // 중복 닫기 방지
	bIsClosing = true;

	if (ViewerWidget)
	{
		ViewerWidget->CloseDocument();
	}

	// 닫기 애니메이션 완료 후 입력 모드 전환 (타이머 기반)
	// ViewerWidget에 Anim_Close가 있으면 그 길이만큼 대기
	float Delay = 0.f;
	if (ViewerWidget && ViewerWidget->Anim_Close)
	{
		Delay = ViewerWidget->Anim_Close->GetEndTime();
	}

	bool bWasFromCollection = bOpenedFromCollection;
	bOpenedFromCollection = false;

	auto FinishClose = [this, bWasFromCollection]()
	{
		bIsClosing = false;

		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (!PC) return;

		if (bWasFromCollection)
		{
			// 수집 UI에서 열었으면 컬렉션 위젯에 포커스 복원
			FInputModeUIOnly InputMode;
			if (CollectionWidget)
			{
				InputMode.SetWidgetToFocus(CollectionWidget->TakeWidget());
			}
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
	};

	if (Delay > 0.f)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda(FinishClose), Delay, false);
	}
	else
	{
		FinishClose();
	}
}

void UDocumentSubsystem::OpenDocumentByIndex(int32 DocIndex, bool bImmediate)
{
	if (bImmediate)
	{
		// 수집 UI 등 일시정지 상태에서 호출 — 지연 없이 바로 열기
		PendingDocIndex = DocIndex;
		OpenDocumentByIndexDeferred();
		return;
	}

	// 같은 프레임에 노티파이와 서류 열기가 동시 호출될 수 있으므로
	// 한 틱 지연 후 노티파이 활성 여부를 재확인
	PendingDocIndex = DocIndex;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			OpenDocumentByIndexDeferred();
		});
	}
}

void UDocumentSubsystem::OpenDocumentByIndexDeferred()
{
	int32 DocIndex = PendingDocIndex;
	if (DocIndex < 0) return;
	PendingDocIndex = -1;

	// 노티파이가 표시 중이면 닫힌 후에 서류 열기
	if (UItemNotifySubsystem* NotifySys = GetLocalPlayer()->GetSubsystem<UItemNotifySubsystem>())
	{
		if (NotifySys->IsNotifyActive())
		{
			PendingDocIndex = DocIndex;
			NotifySys->OnNotifyHidden.AddUObject(this, &UDocumentSubsystem::OnNotifyHiddenOpenPending);
			UE_LOG(LogWard_Zero, Log, TEXT("서류 열기 대기 (인덱스 %d): 노티파이 닫힌 후 실행"), DocIndex);
			return;
		}
	}

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

void UDocumentSubsystem::OnNotifyHiddenOpenPending()
{
	// 델리게이트 해제
	if (UItemNotifySubsystem* NotifySys = GetLocalPlayer()->GetSubsystem<UItemNotifySubsystem>())
	{
		NotifySys->OnNotifyHidden.RemoveAll(this);
	}

	if (PendingDocIndex >= 0)
	{
		int32 DocIdx = PendingDocIndex;
		PendingDocIndex = -1;
		OpenDocumentByIndex(DocIdx);
	}
}

bool UDocumentSubsystem::IsDocumentOpen() const
{
	return bIsClosing || (ViewerWidget && ViewerWidget->IsVisible());
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
	if (IsValid(ViewerWidget) && ViewerWidget->IsInViewport())
	{
		return ViewerWidget;
	}

	// 이전 위젯이 무효화된 경우 (레벨 전환 등) 초기화
	ViewerWidget = nullptr;

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
