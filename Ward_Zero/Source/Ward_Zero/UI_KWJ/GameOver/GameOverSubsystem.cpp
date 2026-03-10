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

		Widget->PlayFadeIn();

		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
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
	if (IsValid(GameOverWidget))
	{
		if (!GameOverWidget->IsInViewport())
		{
			GameOverWidget->AddToViewport(300);
			GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return GameOverWidget;
	}

	if (!GameOverWidgetClass)
	{
		GameOverWidgetClass = LoadClass<UGameOverWidget>(
			nullptr,
			TEXT("/Game/UI/GameOver/WBP_GameOver.WBP_GameOver_C")
		);
	}

	if (!GameOverWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_GameOver not found!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	GameOverWidget = CreateWidget<UGameOverWidget>(PC, GameOverWidgetClass);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport(300);
		GameOverWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return GameOverWidget;
}