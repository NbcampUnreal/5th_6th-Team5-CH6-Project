// GameOverWidget.cpp

#include "UI_KWJ/GameOver/GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/GameOver/GameOverSubsystem.h"
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
	// OnClicked 델리게이트 콜백 내부에서 바로 UI 상태 변경 시
	// InvocationList 오염으로 Ensure 에러 발생 → 다음 틱으로 지연
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (!IsValid(this)) return;

			// GameOver UI 닫기
			UGameOverSubsystem* GameOverSys = GetOwningLocalPlayer()->GetSubsystem<UGameOverSubsystem>();
			if (GameOverSys)
			{
				GameOverSys->HideGameOver();
			}

			// 마지막 세이브 로드
			USaveSubsystem* SaveSys = GetOwningLocalPlayer()->GetSubsystem<USaveSubsystem>();
			if (SaveSys)
			{
				SaveSys->LoadLastSave();
			}
		});
	}
}

void UGameOverWidget::OnLoadSaveClicked()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (!IsValid(this)) return;

			// GameOver UI 먼저 숨기기 (SaveUI zOrder 150 < GameOver 300)
			SetVisibility(ESlateVisibility::Collapsed);

			// Save UI 열기
			USaveSubsystem* SaveSys = GetOwningLocalPlayer()->GetSubsystem<USaveSubsystem>();
			if (SaveSys)
			{
				SaveSys->ShowSaveUI(true); // 게임 오버에서 열었음을 표시
			}
		});
	}
}

void UGameOverWidget::OnReturnToTitleClicked()
{
	// ServerTravel: 같은 PIE 세션 내 월드 교체
	// → LocalPlayer/Subsystem 유지되어 레벨 블루프린트 BeginPlay가 정상 실행됨
	// OpenLevel은 새 PIE 세션을 만들어 BeginPlay 타이밍에 PC가 없어 메뉴가 안 뜸
	if (UWorld* W = GetWorld())
	{
		W->ServerTravel("/Game/UI/Demo", true);
	}
}
