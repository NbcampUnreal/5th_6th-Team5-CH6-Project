// PickupNotifyWidget.h
// 아이템 픽업 알림 컨테이너 — 스크롤 박스에 엔트리 쌓기

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PickupNotifyWidget.generated.h"

class UVerticalBox;
class UPickupEntryWidget;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UPickupNotifyWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 엔트리가 쌓이는 스크롤 박스 */
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ScrollBox_Entries;

	// ══════════════════════════════════════════
	//  기능
	// ══════════════════════════════════════════

	/**
	 * 픽업 알림 엔트리 추가
	 * @param PickupText 표시 텍스트 (예: "힐템 획득 +1")
	 * @param Duration   자동 제거까지의 시간 (초, 기본 3초)
	 */
	void AddEntry(const FText& PickupText, float Duration = 3.0f);

protected:

	/** WBP에서 지정할 엔트리 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "PickupNotify")
	TSubclassOf<UPickupEntryWidget> EntryClass;
};
