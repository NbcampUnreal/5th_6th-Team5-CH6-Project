// DocumentViewerWidget.h
// 서류 뷰어 위젯 C++ 베이스
// UMG Designer에서 이 클래스를 부모로 WBP를 만들고,
// BindWidget 이름에 맞는 위젯들을 배치하면 자동 연결됨

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"	
#include "DocumentViewerWidget.generated.h"

class UDocumentData;
class UScrollBox;
class UTextBlock;
class UButton;
class UImage;
class UCanvasPanel;

UCLASS(BlueprintType, Blueprintable)
class UDocumentViewerWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  UMG BindWidget
	// ══════════════════════════════════════════
	UDocumentViewerWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SetIsFocusable(true);
	}
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* BG_Overlay;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_DocumentBG;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_Content;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_PageContent;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_PageNumber;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Title;

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Back;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_NextPage;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_PrevPage;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_InteractionHint;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* Panel_Hint;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* Panel_Viewer;

	// ══════════════════════════════════════════
	//  애니메이션
	// ══════════════════════════════════════════

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Anim_Open;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Anim_Close;

	// ══════════════════════════════════════════
	//  공개 함수
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Document")
	void OpenDocument(UDocumentData* InDocument);

	UFUNCTION(BlueprintCallable, Category = "Document")
	void CloseDocument();

	UFUNCTION(BlueprintCallable, Category = "Document")
	void TurnPageNext();

	UFUNCTION(BlueprintCallable, Category = "Document")
	void TurnPagePrev();

	UFUNCTION(BlueprintCallable, Category = "Document")
	void ShowHint(const FText& HintText);

	UFUNCTION(BlueprintCallable, Category = "Document")
	void HideHint();

protected:

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(BlueprintReadOnly, Category = "Document")
	UDocumentData* CurrentDocument;

	UPROPERTY(BlueprintReadOnly, Category = "Document")
	int32 CurrentPageIndex = 0;

private:

	void UpdatePageContent();
	void UpdatePageButtons();

	UFUNCTION()
	void OnBackClicked();

	UFUNCTION()
	void OnNextPageClicked();

	UFUNCTION()
	void OnPrevPageClicked();
};
