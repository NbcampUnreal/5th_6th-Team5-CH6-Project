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

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		bIsFocusable = true;
	}

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 시작 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Start;

	/** 설정 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Settings;

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

	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();

	/** 메뉴 숨기고 게임 입력 모드로 전환 */
	void HideMenuAndPlay();
};
