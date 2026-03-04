#include "UI_KWJ/GameClear/GameClearSubsystem.h"
#include "UI_KWJ/GameClear/GameClearWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UGameClearSubsystem::ShowGameClear(float PlayTimeSeconds)
{
	UGameClearWidget* Widget = GetOrCreateWidget();
	if (!Widget) return;

	Widget->SetVisibility(ESlateVisibility::Visible);
	Widget->ShowResult(PlayTimeSeconds);

	if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(Widget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UGameClearSubsystem::HideGameClear()
{
	if (GameClearWidget)
		GameClearWidget->SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
	{
		PC->SetInputMode(FInputModeGameOnly{});
		PC->SetShowMouseCursor(false);
	}
}

bool UGameClearSubsystem::IsGameClearShowing() const
{
	return GameClearWidget && GameClearWidget->IsVisible();
}

UGameClearWidget* UGameClearSubsystem::GetOrCreateWidget()
{
	if (GameClearWidget) return GameClearWidget;

	if (!GameClearWidgetClass)
		GameClearWidgetClass = LoadClass<UGameClearWidget>(nullptr,
			TEXT("/Game/UI/GameClear/WBP_GameClear.WBP_GameClear_C"));

	if (!GameClearWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_GameClear 없음! Content/UI/GameClear/ 확인"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	GameClearWidget = CreateWidget<UGameClearWidget>(PC, GameClearWidgetClass);
	if (GameClearWidget)
	{
		GameClearWidget->AddToViewport(300);
		GameClearWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	return GameClearWidget;
}
