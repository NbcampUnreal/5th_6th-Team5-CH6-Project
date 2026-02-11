// MainMenuSubsystem.cpp

#include "UI_KWJ/MainMenu/MainMenuSubsystem.h"
#include "UI_KWJ/MainMenu/MainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UMainMenuSubsystem::ShowMainMenu()
{
	UMainMenuWidget* Menu = GetOrCreateMenu();
	if (Menu)
	{
		Menu->SetVisibility(ESlateVisibility::Visible);
		Menu->SetKeyboardFocus();

		APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(Menu->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}
}

void UMainMenuSubsystem::HideMainMenu()
{
	if (MenuWidget)
	{
		MenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

bool UMainMenuSubsystem::IsMainMenuOpen() const
{
	return MenuWidget && MenuWidget->IsVisible();
}

UMainMenuWidget* UMainMenuSubsystem::GetOrCreateMenu()
{
	if (MenuWidget) return MenuWidget;

	if (!MenuWidgetClass)
	{
		MenuWidgetClass = LoadClass<UMainMenuWidget>(
			nullptr,
			TEXT("/Game/UI/MainMenu/WBP_MainMenu.WBP_MainMenu_C")
		);
	}

	if (!MenuWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_MainMenu를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	MenuWidget = CreateWidget<UMainMenuWidget>(PC, MenuWidgetClass);
	if (MenuWidget)
	{
		MenuWidget->AddToViewport(200);
	}

	return MenuWidget;
}
