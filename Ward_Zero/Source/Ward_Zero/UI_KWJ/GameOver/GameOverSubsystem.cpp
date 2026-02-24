// GameOverSubsystem.cpp

#include "UI_KWJ/GameOver/GameOverSubsystem.h"
#include "UI_KWJ/GameOver/GameOverWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UGameOverSubsystem::ShowGameOver()
{
	UGameOverWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);

		// 페이드인 연출 시작
		Widget->PlayFadeIn();

		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);

			// 게임 일시정지 (선택)
			// PC->SetPause(true);
		}
	}
}

void UGameOverSubsystem::HideGameOver()
{
	if (GameOverWidget)
	{
		GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

bool UGameOverSubsystem::IsGameOver() const
{
	return GameOverWidget && GameOverWidget->IsVisible();
}

UGameOverWidget* UGameOverSubsystem::GetOrCreateWidget()
{
	if (GameOverWidget) return GameOverWidget;

	if (!GameOverWidgetClass)
	{
		GameOverWidgetClass = LoadClass<UGameOverWidget>(
			nullptr,
			TEXT("/Game/UI/GameOver/WBP_GameOver.WBP_GameOver_C")
		);
	}

	if (!GameOverWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_GameOver를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	GameOverWidget = CreateWidget<UGameOverWidget>(PC, GameOverWidgetClass);
	if (GameOverWidget)
	{
		// 메인메뉴(200)보다 위에 표시
		GameOverWidget->AddToViewport(300);
		GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return GameOverWidget;
}
