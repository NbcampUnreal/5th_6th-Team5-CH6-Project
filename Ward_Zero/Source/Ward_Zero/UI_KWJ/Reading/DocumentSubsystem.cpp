// DocumentSubsystem.cpp

#include "UI_KWJ/Reading/DocumentSubsystem.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "UI_KWJ/Reading/DocumentViewerWidget.h"
#include "Blueprint/UserWidget.h"
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
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
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
		ViewerWidget->AddToViewport(100);
		ViewerWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return ViewerWidget;
}
