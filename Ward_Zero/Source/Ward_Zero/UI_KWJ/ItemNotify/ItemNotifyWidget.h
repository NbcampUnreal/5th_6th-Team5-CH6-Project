// ItemNotifyWidget.h
// 아이템 습득 알림 UI — 아이템 이름, 이미지, 사용 키 표시

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemNotifyWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UItemNotifyWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 아이템 이름 (예: "구급상자") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_ItemName;

	/** 아이템 이미지 */
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Item;

	/** 사용 키 안내 (예: "Q키를 눌러 사용") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_KeyHint;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Close;

	// ══════════════════════════════════════════
	//  기능
	// ══════════════════════════════════════════

	/** 알림 표시 */
	void ShowNotify(const FText& ItemName, UTexture2D* ItemImage, const FText& KeyHint);

	/** 알림 숨기기 */
	void HideNotify();

protected:

	virtual void NativeOnInitialized() override;

private:

	UFUNCTION()
	void OnCloseClicked();

	/** 자동 닫기 타이머 */
	FTimerHandle AutoCloseTimerHandle;

	/** 자동 닫기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Notify")
	float AutoCloseDuration = 5.0f;
};
