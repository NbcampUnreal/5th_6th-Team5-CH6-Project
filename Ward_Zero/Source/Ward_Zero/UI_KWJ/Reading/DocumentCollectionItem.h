// DocumentCollectionItem.h
// 서류 수집 목록의 한 칸 — 이미지 + 이름 + 잠금 표시

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_KWJ/Reading/WardDocumentDataTable.h"
#include "DocumentCollectionItem.generated.h"

class UImage;
class UTextBlock;
class UButton;

DECLARE_DELEGATE_OneParam(FOnDocumentIndexClicked, int32);

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UDocumentCollectionItem : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 인덱스 기반 서류 정보 설정 */
	void SetDocumentInfoFromEntry(const FWardDocumentEntry& Entry, bool bUnlocked);

	/** 클릭 콜백 (인덱스 전달) */
	FOnDocumentIndexClicked OnClicked_Index;

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Thumbnail;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Title;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_PageCount;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Lock;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Item;

protected:

	virtual void NativeOnInitialized() override;

private:

	UFUNCTION()
	void OnItemClicked();

	int32 CachedDocIndex = -1;
	bool bIsUnlocked = false;
};
