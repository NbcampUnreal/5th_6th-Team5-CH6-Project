// LoadingWidget.h
// 로딩 화면 위젯 — 검정 배경 + 메시지 텍스트

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

class UTextBlock;
class UImage;
class UThrobber;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 로딩 메시지 설정 */
	void SetLoadingMessage(const FText& Message);

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 검정 배경 이미지 (전체 화면) */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Background;

	/** 로딩 텍스트 (예: "Loading...") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Loading;

	/** 로딩 스피너 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	UThrobber* LoadingThrobber;

protected:

	virtual void NativeConstruct() override;
};
