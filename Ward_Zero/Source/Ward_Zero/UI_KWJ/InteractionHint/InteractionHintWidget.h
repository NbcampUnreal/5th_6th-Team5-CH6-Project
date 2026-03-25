// InteractionHintWidget.h
// 하단 힌트 메시지 위젯 — 잠긴 문, 사용 불가 등

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionHintWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UInteractionHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 메시지 표시 후 자동 숨김 */
	void ShowMessage(float Duration = 3.0f);

	/** 즉시 숨기기 */
	void HideMessage();

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 메시지 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_Message;

private:

	FTimerHandle AutoHideTimerHandle;
};
