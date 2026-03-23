// MainMenuWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UCanvasPanel;
class USoundBase;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SetIsFocusable(true);
	}

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 시작 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Start;

	/** 불러오기 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Load;

	/** 종료 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Quit;

	/** 게임 타이틀 텍스트 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Title;

	/** 메뉴 전체 패널 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* Panel_Menu;

	// ══════════════════════════════════════════
	//  애니메이션 (선택)
	// ══════════════════════════════════════════

	/** 메뉴 등장 애니메이션 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Anim_MenuIn;

	/** 메뉴 사라짐 애니메이션 (시작 누를 때) */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Anim_MenuOut;

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnLoadClicked();

	UFUNCTION()
	void OnQuitClicked();

	// ── 버튼 호버 콜백 ──
	UFUNCTION() void OnStartHovered();
	UFUNCTION() void OnLoadHovered();
	UFUNCTION() void OnQuitHovered();
	UFUNCTION() void OnStartUnhovered();
	UFUNCTION() void OnLoadUnhovered();
	UFUNCTION() void OnQuitUnhovered();

	/** 메뉴 숨기고 게임 입력 모드로 전환 */
	void HideMenuAndPlay();

	/** 사운드 재생 헬퍼 */
	void PlayUISound(USoundBase* Sound);

	/** 버튼 호버 비주얼 효과 */
	void SetButtonHovered(UButton* Button, bool bHovered);

	// ── 효과음 (WBP에서 할당) ──

	/** 버튼 호버 시 효과음 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* HoverSound;

	/** 버튼 클릭 시 효과음 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ClickSound;
};
