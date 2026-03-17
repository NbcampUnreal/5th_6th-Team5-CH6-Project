// SplashWidget.h
// 게임 시작 로고 스플래시 — 페이드인/아웃 + 클릭 스킵

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplashWidget.generated.h"

class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSplashFinished);

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API USplashWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 스플래시 완료 시 호출 (스킵 포함) */
	UPROPERTY(BlueprintAssignable)
	FOnSplashFinished OnSplashFinished;

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 로고 이미지 */
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Logo;

	/** 검정 배경 (전체 화면) */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Background;

	// ══════════════════════════════════════════
	//  설정
	// ══════════════════════════════════════════

	/** 페이드인 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Splash")
	float FadeInDuration = 1.0f;

	/** 로고 유지 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Splash")
	float HoldDuration = 2.0f;

	/** 페이드아웃 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Splash")
	float FadeOutDuration = 1.0f;

	/** 스플래시 시작 */
	void PlaySplash();

protected:

	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:

	/** 스킵 처리 */
	void SkipSplash();

	/** 페이드 단계 타이머 */
	FTimerHandle FadeTimerHandle;
	void StartFadeIn();
	void StartHold();
	void StartFadeOut();
	void FinishSplash();

	/** 현재 진행 중 여부 */
	bool bIsPlaying = false;
	bool bFinished = false;
};
