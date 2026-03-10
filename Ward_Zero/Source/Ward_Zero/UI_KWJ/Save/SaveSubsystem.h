// SaveSubsystem.h
// 세이브/로드/삭제/목록 관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI_KWJ/Save/SaveTypes.h"          // ← FSaveFileInfo
#include "SaveSubsystem.generated.h"

class UWardSaveGame;
class USaveWidget;

// *** FSaveFileInfo 정의 삭제됨 — SaveTypes.h로 이동 ***

UCLASS()
class WARD_ZERO_API USaveSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ══════════════════════════════════════════
	//  핵심 기능
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(const FString& SlotName);

	/** 마지막 세이브 파일 로드 (게임 오버에서 사용) */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadLastSave();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSave(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	TArray<FSaveFileInfo> GetSaveFileList();

	// ══════════════════════════════════════════
	//  세이브 UI
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Save")
	void ShowSaveUI(bool bFromGameOver = false);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void HideSaveUI();

	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsSaveUIOpen() const;

private:

	UWardSaveGame* CollectCurrentGameState();
	void ApplyGameState(UWardSaveGame* SaveData);

	// 레벨 전환 후 적용 대기 중인 세이브 데이터
	UPROPERTY()
	UWardSaveGame* PendingSaveData = nullptr;

	FDelegateHandle OnLevelLoadedHandle;
	void OnLevelLoaded(UWorld* LoadedWorld);

	// 게임 오버 UI에서 세이브 창을 열었는지 여부
	bool bOpenedFromGameOver = false;
	void CaptureScreenshot(TArray<uint8>& OutPNGData, int32& OutWidth, int32& OutHeight);
	UTexture2D* CreateThumbnailFromPNG(const TArray<uint8>& PNGData, int32 Width, int32 Height);
	FString GenerateSlotName() const;

	UPROPERTY()
	TSubclassOf<USaveWidget> SaveWidgetClass;

	UPROPERTY()
	USaveWidget* SaveWidget;

	USaveWidget* GetOrCreateSaveUI();

	/** 마지막으로 저장한 슬롯 이름 */
	FString LastSaveSlotName;

	/** UI 열기 전 캐시된 스크린샷 */
	TArray<uint8> CachedScreenshotData;
	int32 CachedScreenshotWidth = 0;
	int32 CachedScreenshotHeight = 0;

	static const FString SavePrefix;
};
