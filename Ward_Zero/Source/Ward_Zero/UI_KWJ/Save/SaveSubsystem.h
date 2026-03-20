// SaveSubsystem.h
// 세이브/로드/삭제/목록 관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI_KWJ/Save/SaveTypes.h"
#include "UI_KWJ/Save/WardSaveGame.h"
#include "SaveSubsystem.generated.h"

class UWardSaveGame;
class USaveWidget;
class ULoadWidget;

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
	//  세이브 UI (세이브 포인트에서 사용)
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Save")
	void ShowSaveUI();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void HideSaveUI();

	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsSaveUIOpen() const;

	// ══════════════════════════════════════════
	//  불러오기 UI (ESC 메뉴 / 게임오버에서 사용)
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Save")
	void ShowLoadUI(bool bFromGameOver = false);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void HideLoadUI();

	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsLoadUIOpen() const;

private:

	UWardSaveGame* CollectCurrentGameState();
	void ApplyGameState(UWardSaveGame* SaveData);

	FDelegateHandle OnLevelLoadedHandle;
	void OnLevelLoaded(UWorld* LoadedWorld);

	void CaptureScreenshot(TArray<uint8>& OutPNGData, int32& OutWidth, int32& OutHeight);
	UTexture2D* CreateThumbnailFromPNG(const TArray<uint8>& PNGData, int32 Width, int32 Height);
	FString GenerateSlotName() const;

	// ── 세이브 위젯 ──
	UPROPERTY()
	TSubclassOf<USaveWidget> SaveWidgetClass;

	UPROPERTY()
	USaveWidget* SaveWidget;

	USaveWidget* GetOrCreateSaveUI();

	// ── 불러오기 위젯 ──
	UPROPERTY()
	TSubclassOf<ULoadWidget> LoadWidgetClass;

	UPROPERTY()
	ULoadWidget* LoadWidgetInstance;

	ULoadWidget* GetOrCreateLoadUI();

	FString LastSaveSlotName;

	TArray<uint8> CachedScreenshotData;
	int32 CachedScreenshotWidth = 0;
	int32 CachedScreenshotHeight = 0;

	static const FString SavePrefix;
};
