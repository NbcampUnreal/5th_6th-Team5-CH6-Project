// InteractionHintSubsystem.h
// 잠긴 문 등 상호작용 불가 시 하단 메시지 표시

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "InteractionHintSubsystem.generated.h"

class UInteractionHintWidget;

UCLASS()
class WARD_ZERO_API UInteractionHintSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** 힌트 메시지 표시 (위젯 고정 텍스트 사용) */
	UFUNCTION(BlueprintCallable, Category = "InteractionHint")
	void ShowHint(float Duration = 3.0f);

	/** 힌트 메시지 표시 (커스텀 텍스트) */
	UFUNCTION(BlueprintCallable, Category = "InteractionHint")
	void ShowHintWithText(const FText& Message, float Duration = 3.0f);

	/** 힌트 숨기기 */
	UFUNCTION(BlueprintCallable, Category = "InteractionHint")
	void HideHint();

private:

	UPROPERTY()
	TSubclassOf<UInteractionHintWidget> WidgetClass;

	UPROPERTY()
	UInteractionHintWidget* HintWidget = nullptr;

	UInteractionHintWidget* GetOrCreateWidget();
};
