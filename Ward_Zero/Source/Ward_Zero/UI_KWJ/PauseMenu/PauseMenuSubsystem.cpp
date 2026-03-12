// PauseMenuSubsystem.cpp

#include "UI_KWJ/PauseMenu/PauseMenuSubsystem.h"
#include "UI_KWJ/PauseMenu/PauseMenuWidget.h"
#include "UI_KWJ/GameOver/GameOverSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Ward_Zero.h"

void UPauseMenuSubsystem::TogglePauseMenu()
{
	if (IsPauseMenuOpen())
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void UPauseMenuSubsystem::ShowPauseMenu()
{
	// 게임오버 중이면 ESC 메뉴 안 띄움
	if (UGameOverSubsystem* GameOverSys = GetLocalPlayer()->GetSubsystem<UGameOverSubsystem>())
	{
		if (GameOverSys->IsGameOver()) return;
	}

	UPauseMenuWidget* Widget = GetOrCreateWidget();
	if (!Widget) return;

	// 게임 일시정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	Widget->SetVisibility(ESlateVisibility::Visible);

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(Widget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	UE_LOG(LogWard_Zero, Log, TEXT("일시정지 메뉴 열림"));
}

void UPauseMenuSubsystem::HidePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 게임 재개
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}

	UE_LOG(LogWard_Zero, Log, TEXT("일시정지 메뉴 닫힘"));
}

bool UPauseMenuSubsystem::IsPauseMenuOpen() const
{
	return PauseMenuWidget && PauseMenuWidget->IsVisible();
}

UPauseMenuWidget* UPauseMenuSubsystem::GetOrCreateWidget()
{
	if (IsValid(PauseMenuWidget))
	{
		if (!PauseMenuWidget->IsInViewport())
		{
			PauseMenuWidget->AddToViewport(400);
			PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return PauseMenuWidget;
	}

	if (!PauseMenuWidgetClass)
	{
		PauseMenuWidgetClass = LoadClass<UPauseMenuWidget>(
			nullptr,
			TEXT("/Game/UI/PauseMenu/WBP_PauseMenu.WBP_PauseMenu_C")
		);
	}

	if (!PauseMenuWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_PauseMenu를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	PauseMenuWidget = CreateWidget<UPauseMenuWidget>(PC, PauseMenuWidgetClass);
	if (PauseMenuWidget)
	{
		PauseMenuWidget->AddToViewport(400); // 다른 UI보다 위에
		PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return PauseMenuWidget;
}
