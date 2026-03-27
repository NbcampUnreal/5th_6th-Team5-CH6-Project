// DocumentCollectionWidget.h
// 서류 수집 목록 UI — ESC 메뉴에서 열림
// 수집한 서류는 이미지+이름, 미수집은 잠금 표시

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DocumentCollectionWidget.generated.h"

class UDocumentData;
class UButton;
class UTextBlock;
class UUniformGridPanel;
class UScrollBox;
class UWrapBox;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UDocumentCollectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UDocumentCollectionWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SetIsFocusable(true);
	}

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 서류 목록 스크롤 */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_Documents;

	/** 서류 목록 가로 배치 (ScrollBox 안에 배치) */
	UPROPERTY(meta = (BindWidgetOptional))
	UWrapBox* WrapBox_Documents;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Close;

	/** 제목 텍스트 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Header;

	// ══════════════════════════════════════════
	//  기능
	// ══════════════════════════════════════════

	/** 서류 목록 갱신 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void RefreshDocumentList();

protected:

	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:

	UFUNCTION()
	void OnCloseClicked();

	/** 서류 클릭 시 뷰어 열기 (인덱스 기반) */
	void OnDocumentIndexClicked(int32 DocIndex);

	/** 아이템 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Document")
	TSubclassOf<UUserWidget> DocumentItemClass;
};
