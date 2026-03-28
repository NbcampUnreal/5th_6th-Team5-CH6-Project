// PickupEntryWidget.h
// 아이템 픽업 알림 개별 항목 — 자체 타이머로 스스로 제거

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PickupEntryWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UPickupEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** "힐템 획득 +1" 처럼 전체 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_PickupText;

	// ══════════════════════════════════════════
	//  기능
	// ══════════════════════════════════════════

	/**
	 * 항목 초기화 및 타이머 시작
	 * @param InText   표시할 텍스트 (예: "힐템 획득 +1")
	 * @param Duration 자동 제거까지의 시간 (초)
	 */
	void InitEntry(const FText& InText, float Duration);

protected:

	virtual void NativeDestruct() override;

private:

	FTimerHandle RemoveTimerHandle;

	void RemoveSelf();
};
