// SaveWidget.h
// 세이브/로드 UI 위젯

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "SaveWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UImage;
class UCanvasPanel;
class USaveSlotItem;

/**
 * 세이브/로드 화면 위젯
 * WBP에서 아래 이름으로 위젯을 배치하면 자동 바인딩
 */
UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API USaveWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	USaveWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		bIsFocusable = true;
	}

	// ══════════════════════════════════════════
	//  BindWidget — 필수
	// ══════════════════════════════════════════

	/** 세이브 파일 목록 스크롤 */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_SaveList;

	/** 새로 저장 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_SaveNew;

	/** 닫기 버튼 */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Close;

	// ══════════════════════════════════════════
	//  BindWidget — 선택 (선택된 세이브 상세)
	// ══════════════════════════════════════════

	/** 선택된 세이브 스크린샷 */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Screenshot;

	/** 선택된 세이브 이름 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_SaveName;

	/** 선택된 세이브 시간 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_SaveTime;

	/** 선택된 세이브 레벨 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_LevelName;

	/** 로드 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Load;

	/** 삭제 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Delete;

	/** 덮어쓰기 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BTN_Overwrite;

	// ══════════════════════════════════════════
	//  기능
	// ══════════════════════════════════════════

	/** 세이브 파일 목록 새로고침 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void RefreshSaveList();

	/** 저장 버튼 활성/비활성 (게임오버에서 열 때 비활성화) */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetSaveButtonEnabled(bool bEnabled);

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void OnSaveNewClicked();

	UFUNCTION()
	void OnCloseClicked();

	UFUNCTION()
	void OnLoadClicked();

	UFUNCTION()
	void OnDeleteClicked();

	UFUNCTION()
	void OnOverwriteClicked();

	/** 현재 선택된 슬롯 이름 */
	FString SelectedSlotName;

	/** 목록 캐시 (슬롯 클릭 시 인덱스로 접근) */
	UPROPERTY()
	TArray<FSaveFileInfo> CachedSaveFiles;

	/** 세이브 목록 아이템 위젯 클래스 (블루프린트에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Save")
	TSubclassOf<UUserWidget> SaveSlotItemClass;

	/** 슬롯 선택 시 호출 */
	void OnSlotSelected(const FSaveFileInfo& Info);

	/** 상세 정보 패널 업데이트 */
	void UpdateDetailPanel(const FSaveFileInfo& Info);
};
