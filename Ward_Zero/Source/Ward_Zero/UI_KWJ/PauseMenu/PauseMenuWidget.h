// PauseMenuWidget.h
// 일시정지 메뉴 위젯 — 불러오기, 옵션, 메인 메뉴

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UCanvasPanel;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		bIsFocusable = true;
	}

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 불러오기 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Load;

	/** 서류 수집 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Documents;

	/** 옵션 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Options;

	/** 메인 메뉴 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_MainMenu;

	/** 돌아가기 (게임 재개) 버튼 — 선택 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Resume;

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:

	UFUNCTION() void OnLoadClicked();
	UFUNCTION() void OnDocumentsClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnResumeClicked();

	/** 옵션 위젯 */
	UPROPERTY()
	class UOptionsWidget* OptionsWidget;
};
