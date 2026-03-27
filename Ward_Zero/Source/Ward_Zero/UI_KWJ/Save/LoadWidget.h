// LoadWidget.h
// 불러오기 전용 UI 위젯 (ESC 메뉴 / 게임오버에서 사용)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_KWJ/Save/SaveTypes.h"
#include "LoadWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UImage;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API ULoadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	ULoadWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SetIsFocusable(true);
	}

	// ══════════════════════════════════════════
	//  BindWidget — 필수
	// ══════════════════════════════════════════

	/** 세이브 파일 목록 스크롤 */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_SaveList;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Close;

	// ══════════════════════════════════════════
	//  BindWidgetOptional — 선택된 세이브 상세
	// ══════════════════════════════════════════

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Screenshot;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_SaveName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_SaveTime;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_SaveDate;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_LevelName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Playtime;

	/** 불러오기 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Load;

	/** 삭제 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Delete;

	// ══════════════════════════════════════════
	//  기능
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Load")
	void RefreshSaveList();

	/** 나가기 버튼 표시/숨기기 (게임오버에서 열면 숨김) */
	UFUNCTION(BlueprintCallable, Category = "Load")
	void SetCloseButtonVisible(bool bVisible);

	/** 메인메뉴에서 열렸는지 표시 */
	void SetOpenedFromMainMenu(bool bFromMainMenu) { bOpenedFromMainMenu = bFromMainMenu; }

	/** 게임오버 UI에서 열렸는지 표시 (ESC 시 게임오버로 복귀) */
	void SetOpenedFromGameOver(bool bVal) { bOpenedFromGameOver = bVal; }

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:

	UFUNCTION() void OnCloseClicked();
	UFUNCTION() void OnLoadClicked();
	UFUNCTION() void OnDeleteClicked();

	FString SelectedSlotName;

	UPROPERTY()
	TArray<FSaveFileInfo> CachedSaveFiles;

	UPROPERTY(EditDefaultsOnly, Category = "Load")
	TSubclassOf<UUserWidget> SaveSlotItemClass;

	void OnSlotSelected(const FSaveFileInfo& Info);
	void UpdateDetailPanel(const FSaveFileInfo& Info);

	bool bOpenedFromMainMenu = false;
	bool bOpenedFromGameOver = false;
};
