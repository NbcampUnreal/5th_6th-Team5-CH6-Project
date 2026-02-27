// SaveSlotItem.h
// 세이브 슬롯 목록의 개별 아이템 위젯
// ScrollBox에 동적 생성되며, 클릭 시 부모(SaveWidget)에 알림

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "SaveSlotItem.generated.h"

class UButton;
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnSlotItemClicked, const FSaveFileInfo&);

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API USaveSlotItem : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 슬롯 데이터 설정 */
	void SetSlotInfo(const FSaveFileInfo& InInfo);

	/** 클릭 델리게이트 (SaveWidget이 바인딩) */
	FOnSlotItemClicked OnClicked_SlotItem;

protected:

	virtual void NativeConstruct() override;

	/** 슬롯 표시 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_SlotLabel;

	/** 클릭 영역 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Slot;

private:

	/** 이 슬롯의 세이브 정보 */
	FSaveFileInfo SlotInfo;

	UFUNCTION()
	void OnButtonClicked();
};
