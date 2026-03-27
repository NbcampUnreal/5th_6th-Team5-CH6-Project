// OptionsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsWidget.generated.h"

class USlider;
class UComboBoxString;
class UButton;
class UTextBlock;
class USoundMix;
class USoundClass;

UCLASS()
class WARD_ZERO_API UOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** 메인메뉴에서 열렸을 때 true — 닫아도 UI 모드 유지 */
	bool bIsMainMenuMode = false;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

protected:

	// ══════════════════════════════════════════
	//  오디오
	// ══════════════════════════════════════════

	UPROPERTY(meta = (BindWidget))
	USlider* SLD_MasterVolume;

	UPROPERTY(meta = (BindWidget))
	USlider* SLD_BGMVolume;

	UPROPERTY(meta = (BindWidget))
	USlider* SLD_SFXVolume;

	/** 볼륨 % 표시 (선택) */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_MasterValue;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_BGMValue;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_SFXValue;

	/**
	 * 사운드 믹스 & 클래스 — WBP에서 할당
	 * (에디터에서 SoundMix 1개, SoundClass 3개 생성 필요)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MainSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* BGMSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* SFXSoundClass;

	// ══════════════════════════════════════════
	//  그래픽
	// ══════════════════════════════════════════

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* CMB_Resolution;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* CMB_WindowMode;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* CMB_Quality;

	// ══════════════════════════════════════════
	//  감마 (밝기)
	// ══════════════════════════════════════════

	UPROPERTY(meta = (BindWidget))
	USlider* SLD_Gamma;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_GammaValue;

	// ══════════════════════════════════════════
	//  버튼
	// ══════════════════════════════════════════

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Apply;

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Back;

private:

	// ── 콜백 ──
	UFUNCTION() void OnMasterVolumeChanged(float Value);
	UFUNCTION() void OnBGMVolumeChanged(float Value);
	UFUNCTION() void OnSFXVolumeChanged(float Value);
	UFUNCTION() void OnGammaChanged(float Value);
	UFUNCTION() void OnApplyClicked();
	UFUNCTION() void OnBackClicked();

	// ── 오디오 적용 ──
	void ApplyVolume(USoundClass* InSoundClass, float Volume);

	// ── 그래픽 적용 ──
	void PopulateResolutions();
	void PopulateWindowModes();
	void PopulateQualityLevels();

	// ── 설정 저장/로드 ──
	void LoadSettings();
	void SaveSettings();

	// ── 해상도 목록 ──
	TArray<FIntPoint> AvailableResolutions;
};
