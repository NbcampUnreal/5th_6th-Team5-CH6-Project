// GameOverWidget.cpp

#include "UI_KWJ/GameOver/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Ward_Zero.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

void UGameOverWidget::PlayFadeIn()
{
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

// ══════════════════════════════════════════
//  버튼 콜백
// ══════════════════════════════════════════

void UGameOverWidget::OnLoadLastSaveClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameOver: 마지막 세이브 불러오기"));

	// TODO: SaveSubsystem 연결 후 구현
	// USaveSubsystem* SaveSys = ULocalPlayer::GetSubsystem<USaveSubsystem>(GetOwningLocalPlayer());
	// if (SaveSys) SaveSys->LoadLastSave();
}

void UGameOverWidget::OnLoadSaveClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameOver: 세이브 슬롯 선택"));

	// TODO: 세이브 슬롯 선택 UI 열기
	// USaveSubsystem* SaveSys = ULocalPlayer::GetSubsystem<USaveSubsystem>(GetOwningLocalPlayer());
	// if (SaveSys) SaveSys->ShowLoadScreen();
}

void UGameOverWidget::OnReturnToTitleClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameOver: 타이틀로 돌아가기"));

	// 게임 오버 UI 숨기기
	SetVisibility(ESlateVisibility::Collapsed);

	// 메인 메뉴 레벨로 이동
	UGameplayStatics::OpenLevel(
		this,
		FName(TEXT("/Game/UI/MainMenu/L_MainMenu.L_MainMenu"))
	);
}
