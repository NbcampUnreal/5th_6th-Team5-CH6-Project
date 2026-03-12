

#include "UI_KWJ/MainMenu/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "UI_KWJ/Options/OptionsWidget.h"
#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "Ward_Zero.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// ── 클릭 + 호버 바인딩 ──
	if (BTN_Start)
	{
		BTN_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
		BTN_Start->OnHovered.AddDynamic(this, &UMainMenuWidget::OnStartHovered);
		BTN_Start->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnStartUnhovered);
	}
	if (BTN_Settings)
	{
		BTN_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
		BTN_Settings->OnHovered.AddDynamic(this, &UMainMenuWidget::OnSettingsHovered);
		BTN_Settings->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnSettingsUnhovered);
	}
	if (BTN_Quit)
	{
		BTN_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
		BTN_Quit->OnHovered.AddDynamic(this, &UMainMenuWidget::OnQuitHovered);
		BTN_Quit->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnQuitUnhovered);
	}
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 초기 색상 (살짝 어둡게 — 호버 시 밝아짐)
	if (BTN_Start)    BTN_Start->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	if (BTN_Settings) BTN_Settings->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	if (BTN_Quit)     BTN_Quit->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	if (Anim_MenuIn)
	{
		PlayAnimation(Anim_MenuIn);
	}
}


void UMainMenuWidget::OnStartClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 시작 클릭"));
	PlayUISound(ClickSound);

	if (Anim_MenuOut)
	{
		PlayAnimation(Anim_MenuOut);


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

	SetVisibility(ESlateVisibility::Collapsed);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);

		// 로딩 화면 표시
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (ULoadingScreenSubsystem* LoadingSys = LP->GetSubsystem<ULoadingScreenSubsystem>())
			{
				LoadingSys->ShowLoading(FText::FromString(TEXT("Loading...")));
			}
		}
	}

	// ServerTravel: LocalPlayer/Subsystem 유지 → 로딩 위젯도 유지됨
	// 2초 대기 후 ServerTravel (로딩 화면 표시 시간 확보)
	if (UWorld* W = GetWorld())
	{
		FTimerHandle TravelTimer;
		W->GetTimerManager().SetTimer(TravelTimer, [W]()
		{
			W->ServerTravel("/Game/Level/Maps/L_WardZero", true);
		}, 2.0f, false);
	}
}


void UMainMenuWidget::OnSettingsClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 설정 클릭"));
	PlayUISound(ClickSound);

	if (!OptionsWidget)
	{
		if (!OptionsWidgetClass)
		{
			OptionsWidgetClass = LoadClass<UOptionsWidget>(
				nullptr,
				TEXT("/Game/UI/Option/WBP_Options.WBP_Options_C")
			);
		}

		if (!OptionsWidgetClass)
		{
			UE_LOG(LogWard_Zero, Error, TEXT("WBP_Options를 찾을 수 없습니다!"));
			return;
		}

		APlayerController* PC = GetOwningPlayer();
		if (!PC) return;

		OptionsWidget = CreateWidget<UOptionsWidget>(PC, OptionsWidgetClass);
		if (OptionsWidget)
		{
			OptionsWidget->bIsMainMenuMode = true;
			OptionsWidget->AddToViewport(210); // 메인메뉴(200) 위에
		}
	}

	if (OptionsWidget)
	{
		OptionsWidget->SetVisibility(ESlateVisibility::Visible);
	}
}


void UMainMenuWidget::OnQuitClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 종료 클릭"));
	PlayUISound(ClickSound);

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

// ════════════════════════════════════════════════════════
//  호버 콜백
// ════════════════════════════════════════════════════════

void UMainMenuWidget::OnStartHovered()
{
	PlayUISound(HoverSound);
	SetButtonHovered(BTN_Start, true);
}

void UMainMenuWidget::OnSettingsHovered()
{
	PlayUISound(HoverSound);
	SetButtonHovered(BTN_Settings, true);
}

void UMainMenuWidget::OnQuitHovered()
{
	PlayUISound(HoverSound);
	SetButtonHovered(BTN_Quit, true);
}

void UMainMenuWidget::OnStartUnhovered()
{
	SetButtonHovered(BTN_Start, false);
}

void UMainMenuWidget::OnSettingsUnhovered()
{
	SetButtonHovered(BTN_Settings, false);
}

void UMainMenuWidget::OnQuitUnhovered()
{
	SetButtonHovered(BTN_Quit, false);
}

// ════════════════════════════════════════════════════════
//  사운드 재생
// ════════════════════════════════════════════════════════

void UMainMenuWidget::PlayUISound(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

// ════════════════════════════════════════════════════════
//  버튼 호버 비주얼 — 살짝 커지면서 색이 진해짐
// ════════════════════════════════════════════════════════

void UMainMenuWidget::SetButtonHovered(UButton* Button, bool bHovered)
{
	if (!Button) return;

	if (bHovered)
	{
		// 1.05배 확대
		FWidgetTransform Transform;
		Transform.Scale = FVector2D(1.05f, 1.05f);
		Button->SetRenderTransform(Transform);

		// 색 진해짐 (Tint 밝게)
		Button->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
	else
	{
		// 원래 크기
		FWidgetTransform Transform;
		Transform.Scale = FVector2D(1.0f, 1.0f);
		Button->SetRenderTransform(Transform);

		// 원래 색 (살짝 어둡게)
		Button->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	}
}