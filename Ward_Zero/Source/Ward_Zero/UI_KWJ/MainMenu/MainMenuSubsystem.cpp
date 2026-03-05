// MainMenuSubsystem.cpp

#include "UI_KWJ/MainMenu/MainMenuSubsystem.h"
#include "UI_KWJ/MainMenu/MainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UMainMenuSubsystem::ShowMainMenu()
{
	UE_LOG(LogWard_Zero, Log, TEXT("ShowMainMenu 호출됨"));

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("ShowMainMenu: LocalPlayer 없음 - 다음 틱 재시도"));
		if (UWorld* W = GetWorld())
		{
			W->GetTimerManager().SetTimerForNextTick([this]() { ShowMainMenu(); });
		}
		return;
	}

	APlayerController* PC = LP->GetPlayerController(GetWorld());
	if (!PC)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("ShowMainMenu: PlayerController 없음 - 다음 틱 재시도"));
		if (UWorld* W = GetWorld())
		{
			W->GetTimerManager().SetTimerForNextTick([this]() { ShowMainMenu(); });
		}
		return;
	}

	UMainMenuWidget* Menu = GetOrCreateMenu();
	if (Menu)
	{
		Menu->SetVisibility(ESlateVisibility::Visible);
		Menu->SetKeyboardFocus();

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(Menu->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		UE_LOG(LogWard_Zero, Log, TEXT("ShowMainMenu: 메뉴 표시 완료"));
	}
	else
	{
		UE_LOG(LogWard_Zero, Error, TEXT("ShowMainMenu: 위젯 생성 실패"));
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
	if (IsValid(MenuWidget))
	{
		// ServerTravel 후 뷰포트에서 분리됐으면 다시 추가
		if (!MenuWidget->IsInViewport())
		{
			MenuWidget->AddToViewport(200);
		}
		return MenuWidget;
	}

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
	if (!PC)
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("GetOrCreateMenu: PlayerController가 아직 없음 - 한 틱 뒤 재시도"));
		// PC가 없으면 다음 틱에 ShowMainMenu 재호출
		if (UWorld* World = GetLocalPlayer()->GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick([this]()
			{
				ShowMainMenu();
			});
		}
		return nullptr;
	}

	MenuWidget = CreateWidget<UMainMenuWidget>(PC, MenuWidgetClass);
	if (MenuWidget)
	{
		MenuWidget->AddToViewport(200);
	}

	return MenuWidget;
}
