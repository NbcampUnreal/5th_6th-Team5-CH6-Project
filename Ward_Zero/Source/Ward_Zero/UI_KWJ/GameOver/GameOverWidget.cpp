// GameOverWidget.cpp

#include "UI_KWJ/GameOver/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/GameOver/GameOverSubsystem.h"
#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "Ward_Zero.h"

void UGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Show/Hide 반복 시 NativeConstruct가 재호출되어 중복 바인딩 발생
	// → 한 번만 호출되는 NativeOnInitialized에서 바인딩
	if (BTN_LoadLastSave)
	{
		BTN_LoadLastSave->OnClicked.AddDynamic(this, &UGameOverWidget::OnLoadLastSaveClicked);
	}

	if (BTN_LoadSave)
	{
		BTN_LoadSave->OnClicked.AddDynamic(this, &UGameOverWidget::OnLoadSaveClicked);
	}

	if (BTN_ReturnToTitle)
	{
		BTN_ReturnToTitle->OnClicked.AddDynamic(this, &UGameOverWidget::OnReturnToTitleClicked);
	}
}

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGameOverWidget::PlayFadeIn()
{
	UpdateButtonAvailability();

	if (Anim_FadeIn)
	{
		PlayAnimation(Anim_FadeIn);
	}
	else
	{
		// 애니메이션 미설정 시 즉시 표시
		if (IMG_Vignette) IMG_Vignette->SetRenderOpacity(1.0f);
		if (IMG_BlackOverlay) IMG_BlackOverlay->SetRenderOpacity(0.7f);
		if (TXT_GameOver) TXT_GameOver->SetRenderOpacity(1.0f);
		if (Panel_Buttons) Panel_Buttons->SetRenderOpacity(1.0f);
	}
}

void UGameOverWidget::UpdateButtonAvailability()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSys = LP->GetSubsystem<USaveSubsystem>();
	bool bHasSaves = SaveSys && SaveSys->GetSaveFileList().Num() > 0;

	if (BTN_LoadLastSave) BTN_LoadLastSave->SetIsEnabled(bHasSaves);
	if (BTN_LoadSave)     BTN_LoadSave->SetIsEnabled(bHasSaves);
}

// ══════════════════════════════════════════
//  버튼 콜백
// ══════════════════════════════════════════

void UGameOverWidget::OnLoadLastSaveClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameOver: 마지막 세이브 불러오기 클릭"));

	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP) return;

	// GameOver UI만 숨기기 (일시정지는 유지 — LoadGame에서 ServerTravel 시 해제)
	SetVisibility(ESlateVisibility::Collapsed);

	// 마지막 세이브 로드
	USaveSubsystem* SaveSys = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSys)
	{
		SaveSys->LoadLastSave();
	}
}

void UGameOverWidget::OnLoadSaveClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameOver: 불러오기 클릭"));

	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP) return;

	// GameOver UI 숨기기 (일시정지 유지)
	SetVisibility(ESlateVisibility::Collapsed);

	// Load UI 열기
	USaveSubsystem* SaveSys = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSys)
	{
		SaveSys->ShowLoadUI(true);
	}
}

void UGameOverWidget::OnReturnToTitleClicked()
{
	if (UWorld* W = GetWorld())
	{
		// 로딩 화면 표시
		if (ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			if (ULoadingScreenSubsystem* LoadingSys = LP->GetSubsystem<ULoadingScreenSubsystem>())
			{
				LoadingSys->ShowLoading(FText::FromString(TEXT("Loading...")));
			}
		}

		W->ServerTravel("/Game/UI/Demo", true);
	}
}
