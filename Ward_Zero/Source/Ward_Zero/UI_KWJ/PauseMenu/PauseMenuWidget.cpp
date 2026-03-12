// PauseMenuWidget.cpp

#include "UI_KWJ/PauseMenu/PauseMenuWidget.h"
#include "UI_KWJ/PauseMenu/PauseMenuSubsystem.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/Options/OptionsWidget.h"
#include "UI_KWJ/Loading/LoadingScreenSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Load)     BTN_Load->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnLoadClicked);
	if (BTN_Options)  BTN_Options->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnOptionsClicked);
	if (BTN_MainMenu) BTN_MainMenu->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
	if (BTN_Resume)   BTN_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
}

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// ESC로 메뉴 닫기
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnResumeClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ════════════════════════════════════════════════════════
//  버튼 콜백
// ════════════════════════════════════════════════════════

void UPauseMenuWidget::OnResumeClicked()
{
	UPauseMenuSubsystem* PauseSys = GetOwningLocalPlayer()->GetSubsystem<UPauseMenuSubsystem>();
	if (PauseSys)
	{
		PauseSys->HidePauseMenu();
	}
}

void UPauseMenuWidget::OnLoadClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("일시정지: 불러오기"));

	// 일시정지 해제 후 세이브 UI 열기
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	SetVisibility(ESlateVisibility::Collapsed);

	USaveSubsystem* SaveSys = GetOwningLocalPlayer()->GetSubsystem<USaveSubsystem>();
	if (SaveSys)
	{
		SaveSys->ShowSaveUI(false);
	}
}

void UPauseMenuWidget::OnOptionsClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("일시정지: 옵션"));

	if (!OptionsWidget)
	{
		TSubclassOf<UOptionsWidget> OptionsClass = LoadClass<UOptionsWidget>(
			nullptr,
			TEXT("/Game/UI/Option/WBP_Options.WBP_Options_C")
		);

		if (!OptionsClass)
		{
			UE_LOG(LogWard_Zero, Error, TEXT("WBP_Options를 찾을 수 없습니다!"));
			return;
		}

		APlayerController* PC = GetOwningPlayer();
		if (!PC) return;

		OptionsWidget = CreateWidget<UOptionsWidget>(PC, OptionsClass);
		if (OptionsWidget)
		{
			OptionsWidget->AddToViewport(410); // 일시정지 메뉴(400) 위에
		}
	}

	if (OptionsWidget)
	{
		OptionsWidget->bIsMainMenuMode = true; // 닫아도 UI 모드 유지
		OptionsWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("일시정지: 메인 메뉴로"));

	// 일시정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	SetVisibility(ESlateVisibility::Collapsed);

	// 로딩 화면 표시
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (ULoadingScreenSubsystem* LoadingSys = LP->GetSubsystem<ULoadingScreenSubsystem>())
		{
			LoadingSys->ShowLoading(FText::FromString(TEXT("Loading...")));
		}
	}

	// 메인 메뉴로 이동
	if (UWorld* W = GetWorld())
	{
		W->ServerTravel("/Game/UI/Demo", true);
	}
}
