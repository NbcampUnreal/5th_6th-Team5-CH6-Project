// OptionsWidget.cpp

#include "UI_KWJ/Options/OptionsWidget.h"
#include "WardGameInstanceSubsystem.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioDevice.h"
#include "Ward_Zero.h"

// 설정 저장 섹션/키
static const TCHAR* AudioSection = TEXT("WardZero.Audio");
static const TCHAR* DisplaySection = TEXT("WardZero.Display");

void UOptionsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// ── 슬라이더 콜백 ──
	if (SLD_MasterVolume) SLD_MasterVolume->OnValueChanged.AddDynamic(this, &UOptionsWidget::OnMasterVolumeChanged);
	if (SLD_BGMVolume)    SLD_BGMVolume->OnValueChanged.AddDynamic(this, &UOptionsWidget::OnBGMVolumeChanged);
	if (SLD_SFXVolume)    SLD_SFXVolume->OnValueChanged.AddDynamic(this, &UOptionsWidget::OnSFXVolumeChanged);
	if (SLD_Gamma)        SLD_Gamma->OnValueChanged.AddDynamic(this, &UOptionsWidget::OnGammaChanged);

	// ── 버튼 콜백 ──
	if (BTN_Apply) BTN_Apply->OnClicked.AddDynamic(this, &UOptionsWidget::OnApplyClicked);
	if (BTN_Back)  BTN_Back->OnClicked.AddDynamic(this, &UOptionsWidget::OnBackClicked);
}

void UOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ── 콤보박스 채우기 ──
	PopulateResolutions();
	PopulateWindowModes();
	PopulateQualityLevels();

	// ── 저장된 설정 불러오기 ──
	LoadSettings();
}

// ════════════════════════════════════════════════════════
//  오디오
// ════════════════════════════════════════════════════════

void UOptionsWidget::OnMasterVolumeChanged(float Value)
{
	if (UWorld* World = GetWorld())
	{
		FAudioDeviceHandle AudioDevice = World->GetAudioDevice();
		if (AudioDevice.IsValid())
		{
			AudioDevice->SetTransientPrimaryVolume(Value);
		}

		// GameInstance에 저장 (레벨 전환 시 유지)
		if (UWardGameInstanceSubsystem* GI = World->GetGameInstance()->GetSubsystem<UWardGameInstanceSubsystem>())
		{
			GI->MasterVolume = Value;
		}
	}

	if (TXT_MasterValue)
	{
		TXT_MasterValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f))));
	}
}

void UOptionsWidget::OnBGMVolumeChanged(float Value)
{
	ApplyVolume(BGMSoundClass, Value);

	if (UWorld* World = GetWorld())
	{
		if (UWardGameInstanceSubsystem* GI = World->GetGameInstance()->GetSubsystem<UWardGameInstanceSubsystem>())
		{
			GI->BGMVolume = Value;
		}
	}

	if (TXT_BGMValue)
	{
		TXT_BGMValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f))));
	}
}

void UOptionsWidget::OnSFXVolumeChanged(float Value)
{
	ApplyVolume(SFXSoundClass, Value);

	if (UWorld* World = GetWorld())
	{
		if (UWardGameInstanceSubsystem* GI = World->GetGameInstance()->GetSubsystem<UWardGameInstanceSubsystem>())
		{
			GI->SFXVolume = Value;
		}
	}

	if (TXT_SFXValue)
	{
		TXT_SFXValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f))));
	}
}

void UOptionsWidget::ApplyVolume(USoundClass* InSoundClass, float Volume)
{
	if (!MainSoundMix || !InSoundClass)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("옵션: SoundMix 또는 SoundClass 미할당"));
		return;
	}

	UGameplayStatics::PushSoundMixModifier(this, MainSoundMix);
	UGameplayStatics::SetSoundMixClassOverride(this, MainSoundMix, InSoundClass, Volume, 1.0f, 0.0f);
}

// ════════════════════════════════════════════════════════
//  감마 (밝기)
// ════════════════════════════════════════════════════════

void UOptionsWidget::OnGammaChanged(float Value)
{
	float GammaValue = FMath::Lerp(1.5f, 3.5f, Value);

	if (GEngine)
	{
		GEngine->DisplayGamma = GammaValue;
	}

	if (UWorld* World = GetWorld())
	{
		if (UWardGameInstanceSubsystem* GI = World->GetGameInstance()->GetSubsystem<UWardGameInstanceSubsystem>())
		{
			GI->Gamma = Value;
		}
	}

	if (TXT_GammaValue)
	{
		TXT_GammaValue->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), GammaValue)));
	}
}

// ════════════════════════════════════════════════════════
//  그래픽 — 콤보박스 채우기
// ════════════════════════════════════════════════════════

void UOptionsWidget::PopulateResolutions()
{
	if (!CMB_Resolution) return;

	CMB_Resolution->ClearOptions();
	AvailableResolutions.Empty();

	TArray<FIntPoint> Resolutions;
	// 일반적인 해상도 목록
	Resolutions.Add(FIntPoint(1280, 720));
	Resolutions.Add(FIntPoint(1366, 768));
	Resolutions.Add(FIntPoint(1600, 900));
	Resolutions.Add(FIntPoint(1920, 1080));
	Resolutions.Add(FIntPoint(2560, 1440));
	Resolutions.Add(FIntPoint(3840, 2160));

	for (const FIntPoint& Res : Resolutions)
	{
		FString Label = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
		CMB_Resolution->AddOption(Label);
		AvailableResolutions.Add(Res);
	}

	// 현재 해상도 선택
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	FIntPoint CurrentRes = Settings->GetScreenResolution();

	for (int32 i = 0; i < AvailableResolutions.Num(); i++)
	{
		if (AvailableResolutions[i] == CurrentRes)
		{
			CMB_Resolution->SetSelectedIndex(i);
			break;
		}
	}
}

void UOptionsWidget::PopulateWindowModes()
{
	if (!CMB_WindowMode) return;

	CMB_WindowMode->ClearOptions();
	CMB_WindowMode->AddOption(TEXT("전체 화면"));
	CMB_WindowMode->AddOption(TEXT("창 모드"));
	CMB_WindowMode->AddOption(TEXT("전체 창 모드"));

	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	EWindowMode::Type CurrentMode = Settings->GetFullscreenMode();

	switch (CurrentMode)
	{
	case EWindowMode::Fullscreen:           CMB_WindowMode->SetSelectedIndex(0); break;
	case EWindowMode::Windowed:             CMB_WindowMode->SetSelectedIndex(1); break;
	case EWindowMode::WindowedFullscreen:   CMB_WindowMode->SetSelectedIndex(2); break;
	default:                                CMB_WindowMode->SetSelectedIndex(0); break;
	}
}

void UOptionsWidget::PopulateQualityLevels()
{
	if (!CMB_Quality) return;

	CMB_Quality->ClearOptions();
	CMB_Quality->AddOption(TEXT("낮음"));
	CMB_Quality->AddOption(TEXT("중간"));
	CMB_Quality->AddOption(TEXT("높음"));
	CMB_Quality->AddOption(TEXT("울트라"));

	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	int32 Quality = Settings->GetOverallScalabilityLevel();

	// -1이면 커스텀, 기본 높음으로
	if (Quality < 0 || Quality > 3) Quality = 2;
	CMB_Quality->SetSelectedIndex(Quality);
}

// ════════════════════════════════════════════════════════
//  적용 / 닫기
// ════════════════════════════════════════════════════════

void UOptionsWidget::OnApplyClicked()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	// ── 해상도 ──
	int32 ResIndex = CMB_Resolution->GetSelectedIndex();
	if (AvailableResolutions.IsValidIndex(ResIndex))
	{
		FIntPoint Res = AvailableResolutions[ResIndex];
		Settings->SetScreenResolution(Res);
	}

	// ── 창 모드 ──
	int32 WinIndex = CMB_WindowMode->GetSelectedIndex();
	switch (WinIndex)
	{
	case 0: Settings->SetFullscreenMode(EWindowMode::Fullscreen); break;
	case 1: Settings->SetFullscreenMode(EWindowMode::Windowed); break;
	case 2: Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen); break;
	}

	// ── 그래픽 품질 ──
	int32 QualityIndex = CMB_Quality->GetSelectedIndex();
	Settings->SetOverallScalabilityLevel(QualityIndex);

	// ── 적용 & 저장 ──
	Settings->ApplySettings(false);
	Settings->SaveSettings();

	// ── 오디오 + 감마 저장 ──
	SaveSettings();

	UE_LOG(LogWard_Zero, Log, TEXT("옵션 설정 적용 완료"));
}

void UOptionsWidget::OnBackClicked()
{
	SaveSettings();
	SetVisibility(ESlateVisibility::Collapsed);

	// 메인메뉴에서 열렸으면 UI 모드 유지 (메인메뉴가 아직 떠있으므로)
	if (!bIsMainMenuMode)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(false);
		}
	}
}

// ════════════════════════════════════════════════════════
//  설정 저장 / 로드 (INI 파일)
// ════════════════════════════════════════════════════════

void UOptionsWidget::SaveSettings()
{
	float MasterVol = SLD_MasterVolume ? SLD_MasterVolume->GetValue() : 1.0f;
	float BGMVol    = SLD_BGMVolume    ? SLD_BGMVolume->GetValue()    : 1.0f;
	float SFXVol    = SLD_SFXVolume    ? SLD_SFXVolume->GetValue()    : 1.0f;
	float GammaVal  = SLD_Gamma        ? SLD_Gamma->GetValue()        : 0.35f;

	GConfig->SetFloat(AudioSection,   TEXT("MasterVolume"), MasterVol, GGameUserSettingsIni);
	GConfig->SetFloat(AudioSection,   TEXT("BGMVolume"),    BGMVol,    GGameUserSettingsIni);
	GConfig->SetFloat(AudioSection,   TEXT("SFXVolume"),    SFXVol,    GGameUserSettingsIni);
	GConfig->SetFloat(DisplaySection, TEXT("Gamma"),        GammaVal,  GGameUserSettingsIni);

	GConfig->Flush(false, GGameUserSettingsIni);

	UE_LOG(LogWard_Zero, Log, TEXT("옵션 저장: Master=%.0f%% BGM=%.0f%% SFX=%.0f%% Gamma=%.1f"),
		MasterVol * 100.f, BGMVol * 100.f, SFXVol * 100.f, FMath::Lerp(1.5f, 3.5f, GammaVal));
}

void UOptionsWidget::LoadSettings()
{
	float MasterVol = 1.0f, BGMVol = 1.0f, SFXVol = 1.0f, GammaVal = 0.35f;

	GConfig->GetFloat(AudioSection,   TEXT("MasterVolume"), MasterVol, GGameUserSettingsIni);
	GConfig->GetFloat(AudioSection,   TEXT("BGMVolume"),    BGMVol,    GGameUserSettingsIni);
	GConfig->GetFloat(AudioSection,   TEXT("SFXVolume"),    SFXVol,    GGameUserSettingsIni);
	GConfig->GetFloat(DisplaySection, TEXT("Gamma"),        GammaVal,  GGameUserSettingsIni);

	// 슬라이더 값 설정 (콜백 자동 호출)
	if (SLD_MasterVolume) SLD_MasterVolume->SetValue(MasterVol);
	if (SLD_BGMVolume)    SLD_BGMVolume->SetValue(BGMVol);
	if (SLD_SFXVolume)    SLD_SFXVolume->SetValue(SFXVol);
	if (SLD_Gamma)        SLD_Gamma->SetValue(GammaVal);

	// 슬라이더 SetValue는 콜백 안 부를 수 있으니 수동 적용
	OnMasterVolumeChanged(MasterVol);
	OnBGMVolumeChanged(BGMVol);
	OnSFXVolumeChanged(SFXVol);
	OnGammaChanged(GammaVal);
}
