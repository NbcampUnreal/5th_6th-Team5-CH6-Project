// GameOverWidget.h
// 게임 오버 화면 위젯
// - 주변부에서 안쪽으로 번지는 페이드인 연출
// - 마지막 세이브 / 불러오기 / 타이틀로 버튼

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UCanvasPanel;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UGameOverWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		bIsFocusable = true;
	}

	/** 페이드인 연출 재생 */
	void PlayFadeIn();

	// ══════════════════════════════════════════
	//  BindWidget - 버튼
	// ══════════════════════════════════════════

	/** 마지막 세이브 불러오기 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_LoadLastSave;

	/** 불러오기 (세이브 슬롯 선택) */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_LoadSave;

	/** 타이틀 화면으로 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_ReturnToTitle;

	// ══════════════════════════════════════════
	//  BindWidget - 연출용 (선택)
	// ══════════════════════════════════════════

	/** 전체 배경 오버레이 (검정/피 비네팅) */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Vignette;

	/** 검정 페이드 오버레이 */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_BlackOverlay;

	/** 버튼들을 감싸는 패널 */
	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* Panel_Buttons;

	/** GAME OVER 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_GameOver;

	// ══════════════════════════════════════════
	//  애니메이션 (선택)
	// ══════════════════════════════════════════

	/**
	 * 게임 오버 등장 애니메이션
	 * WBP에서 만들 때:
	 * 0.0~1.0s: IMG_Vignette 알파 0→1 (가장자리 피 번짐)
	 * 0.5~1.5s: IMG_BlackOverlay 알파 0→0.7 (어둠 깔림)
	 * 1.5~2.0s: TXT_GameOver + Panel_Buttons 알파 0→1
	 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Anim_FadeIn;

protected:

	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void OnLoadLastSaveClicked();

	UFUNCTION()
	void OnLoadSaveClicked();

	UFUNCTION()
	void OnReturnToTitleClicked();
};
