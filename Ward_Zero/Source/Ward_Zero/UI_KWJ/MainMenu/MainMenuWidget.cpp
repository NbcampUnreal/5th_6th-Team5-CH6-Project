// MainMenuWidget.cpp

#include "UI_KWJ/MainMenu/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Start)
	{
		BTN_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}
	if (BTN_Settings)
	{
		BTN_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}
	if (BTN_Quit)
	{
		BTN_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	// 메뉴 등장 시 UI 입력 모드 + 마우스 커서
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	// 등장 애니메이션
	if (Anim_MenuIn)
	{
		PlayAnimation(Anim_MenuIn);
	}
}

// ────────────────────────────────────────────
//  시작
// ────────────────────────────────────────────

void UMainMenuWidget::OnStartClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 시작 클릭"));

	if (Anim_MenuOut)
	{
		PlayAnimation(Anim_MenuOut);

		// 애니메이션 끝나면 숨기기
		float AnimLength = Anim_MenuOut->GetEndTime();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			HideMenuAndPlay();
		}, AnimLength, false);
	}
	else
	{
		HideMenuAndPlay();
	}
}

void UMainMenuWidget::HideMenuAndPlay()
{
	// UI 숨기기
	SetVisibility(ESlateVisibility::Collapsed);

	// 게임 입력 모드로 전환
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}

	// TODO: 나중에 여기서 레벨 전환
	// UGameplayStatics::OpenLevel(this, FName("PlayLevel"));
}

// ────────────────────────────────────────────
//  설정
// ────────────────────────────────────────────

void UMainMenuWidget::OnSettingsClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 설정 클릭 (미구현)"));

	// TODO: 설정 위젯 열기
}

// ────────────────────────────────────────────
//  종료
// ────────────────────────────────────────────

void UMainMenuWidget::OnQuitClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 종료 클릭"));

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(
			GetWorld(),
			PC,
			EQuitPreference::Quit,
			false
		);
	}
}
