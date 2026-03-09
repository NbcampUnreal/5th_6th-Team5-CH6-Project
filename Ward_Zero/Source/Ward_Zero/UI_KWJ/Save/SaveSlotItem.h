// SaveSlotItem.h
// 세이브 슬롯 목록의 개별 아이템 위젯
// ScrollBox / ListView에 동적 생성되며, 클릭 시 부모(SaveWidget)에 알림

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_KWJ/Save/SaveTypes.h"
#include "SaveSlotItem.generated.h"

class UButton;
class UTextBlock;
class UImage;

// 슬롯 클릭 시 부모에게 전달하는 델리게이트
DECLARE_DELEGATE_OneParam(FOnSlotItemClicked, const FSaveFileInfo&);

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API USaveSlotItem : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 외부에서 슬롯 데이터를 주입하여 UI 갱신 */
	void SetSlotInfo(const FSaveFileInfo& InInfo);

	/** 클릭 델리게이트 — SaveWidget이 바인딩하여 선택 처리 */
	FOnSlotItemClicked OnClicked_SlotItem;

protected:

	virtual void NativeConstruct() override;

	// ── BindWidget: WBP 블루프린트에 동일 이름의 위젯 필요 ──

	/** 슬롯 표시 이름 (DisplayName) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_SlotLabel;

	/** 저장 날짜/시간 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_DateTime;

	/** 레벨(맵) 이름 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_LevelName;

	/** 세이브 시점 스크린샷 썸네일 */
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Thumbnail;

	/** 누적 플레이 타임 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_Playtime;

	/** 슬롯 전체를 감싸는 클릭 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Slot;

private:

	/** 이 슬롯이 표현하는 세이브 정보 캐싱 */
	FSaveFileInfo SlotInfo;

	/** BTN_Slot 클릭 콜백 */
	UFUNCTION()
	void OnButtonClicked();
};
