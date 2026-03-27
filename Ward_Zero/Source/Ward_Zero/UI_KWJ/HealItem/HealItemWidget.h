// HealItemWidget.h
// 힐템 보유 수량 표시 위젯

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealItemWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UHealItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 힐템 수량 갱신 */
	void UpdateHealCount(int32 Current, int32 Max);

	/** 표시/숨기기 */
	void SetHealUIVisible(bool bVisible);

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 수량 텍스트 (예: "3 / 5") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_HealCount;

	/** 힐템 아이콘 */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_HealIcon;
};
