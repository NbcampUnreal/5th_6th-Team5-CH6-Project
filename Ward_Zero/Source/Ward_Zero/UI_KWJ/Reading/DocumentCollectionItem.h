// DocumentCollectionItem.h
// 서류 수집 목록의 한 칸 — 이미지 + 이름 + 잠금/페이지 표시

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DocumentCollectionItem.generated.h"

class UDocumentData;
class UImage;
class UTextBlock;
class UButton;

DECLARE_DELEGATE_OneParam(FOnDocumentItemClicked, UDocumentData*);

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UDocumentCollectionItem : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 서류 데이터 설정 */
	void SetDocumentInfo(UDocumentData* InDocument, bool bUnlocked);

	/** 클릭 콜백 */
	FOnDocumentItemClicked OnClicked_Item;

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 서류 배경/썸네일 이미지 */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Thumbnail;

	/** 서류 제목 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Title;

	/** 페이지 수 표시 (예: "Pages: 1/3") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_PageCount;

	/** 잠금 아이콘 */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Lock;

	/** 클릭 버튼 (전체 영역) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Item;

protected:

	virtual void NativeOnInitialized() override;

private:

	UFUNCTION()
	void OnItemClicked();

	UPROPERTY()
	UDocumentData* CachedDocument = nullptr;

	bool bIsUnlocked = false;
};
