

#include "UI_KWJ/MainMenu/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "WardGameInstanceSubsystem.h"
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
	if (BTN_Load)
	{
		BTN_Load->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadClicked);
		BTN_Load->OnHovered.AddDynamic(this, &UMainMenuWidget::OnLoadHovered);
		BTN_Load->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnLoadUnhovered);
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
	if (BTN_Load)     BTN_Load->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
	if (BTN_Quit)     BTN_Quit->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));

	// 세이브 파일 없으면 불러오기 비활성화
	if (BTN_Load)
	{
		APlayerController* TempPC = GetOwningPlayer();
		if (TempPC)
		{
			if (ULocalPlayer* LP = TempPC->GetLocalPlayer())
			{
				USaveSubsystem* SaveSys = LP->GetSubsystem<USaveSubsystem>();
				if (SaveSys)
				{
					bool bHasSaves = SaveSys->GetSaveFileList().Num() > 0;
					BTN_Load->SetIsEnabled(bHasSaves);
					if (!bHasSaves)
					{
						// 회색 + 반투명으로 비활성화 표시
						BTN_Load->SetColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 0.4f));
					}
				}
			}
		}
	}

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
		// 새 게임 — 인스턴스 데이터 초기화
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UGameInstance* GI = LP->GetGameInstance())
			{
				if (UWardGameInstanceSubsystem* SaveGI = GI->GetSubsystem<UWardGameInstanceSubsystem>())
				{
					SaveGI->ResetForNewGame();
				}
			}
		}

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);

		// 로딩 화면 표시
		if (ULocalPlayer* LP2 = PC->GetLocalPlayer())
		{
			if (ULoadingScreenSubsystem* LoadingSys = LP2->GetSubsystem<ULoadingScreenSubsystem>())
			{
				LoadingSys->ShowLoading(FText::FromString(TEXT("Loading...")));
			}
		}
	}

	// MoviePlayer가 로딩 화면을 레벨 전환 중에도 유지해줌
	if (UWorld* W = GetWorld())
	{
		W->ServerTravel("/Game/Level/Maps/L_WardZero", true);
	}
}

void UMainMenuWidget::OnLoadClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 불러오기 클릭"));
	PlayUISound(ClickSound);

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	USaveSubsystem* SaveSys = LP->GetSubsystem<USaveSubsystem>();
	if (SaveSys)
	{
		SaveSys->ShowLoadUI(false, true);
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

void UMainMenuWidget::OnLoadHovered()
{
	if (BTN_Load && !BTN_Load->GetIsEnabled()) return;
	PlayUISound(HoverSound);
	SetButtonHovered(BTN_Load, true);
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

void UMainMenuWidget::OnLoadUnhovered()
{
	SetButtonHovered(BTN_Load, false);
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