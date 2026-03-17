// SplashSubsystem.cpp

#include "UI_KWJ/Splash/SplashSubsystem.h"
#include "UI_KWJ/Splash/SplashWidget.h"
#include "UI_KWJ/MainMenu/MainMenuSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void USplashSubsystem::ShowSplash()
{
	// 세션당 1회만 표시 (타이틀로 돌아올 때는 스킵)
	if (bHasShownSplash)
	{
		// 바로 메인메뉴 표시
		if (UMainMenuSubsystem* MenuSys = GetLocalPlayer()->GetSubsystem<UMainMenuSubsystem>())
		{
			MenuSys->ShowMainMenu();
		}
		return;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return;

	if (!SplashWidget)
	{
		TSubclassOf<USplashWidget> SplashClass = LoadClass<USplashWidget>(
			nullptr,
			TEXT("/Game/UI/Splash/WBP_Splash.WBP_Splash_C")
		);

		if (!SplashClass)
		{
			UE_LOG(LogWard_Zero, Error, TEXT("WBP_Splash를 찾을 수 없습니다!"));
			// 스플래시 없으면 바로 메인메뉴
			if (UMainMenuSubsystem* MenuSys = GetLocalPlayer()->GetSubsystem<UMainMenuSubsystem>())
			{
				MenuSys->ShowMainMenu();
			}
			return;
		}

		SplashWidget = CreateWidget<USplashWidget>(PC, SplashClass);
		if (SplashWidget)
		{
			SplashWidget->AddToViewport(500); // 모든 UI 위에
			SplashWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			SplashWidget->OnSplashFinished.AddDynamic(this, &USplashSubsystem::OnSplashFinished);
		}
	}

	if (SplashWidget)
	{
		// UI 모드 (클릭/키 입력 받기 위해)
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(SplashWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false); // 스플래시 중 마우스 숨기기

		// PIE에서 뷰포트 포커스 안정화를 위해 0.1초 후 시작
		if (UWorld* World = GetWorld())
		{
			FTimerHandle DelayHandle;
			World->GetTimerManager().SetTimer(DelayHandle, [this]()
			{
				if (SplashWidget)
				{
					SplashWidget->PlaySplash();
				}
			}, 0.1f, false);
		}

		bHasShownSplash = true;
		UE_LOG(LogWard_Zero, Log, TEXT("스플래시 표시 시작"));
	}
}

bool USplashSubsystem::IsSplashPlaying() const
{
	return SplashWidget && SplashWidget->IsVisible();
}

void USplashSubsystem::OnSplashFinished()
{
	UE_LOG(LogWard_Zero, Log, TEXT("스플래시 완료 → 메인메뉴 표시"));

	// 스플래시 완료 → 메인메뉴 표시
	if (UMainMenuSubsystem* MenuSys = GetLocalPlayer()->GetSubsystem<UMainMenuSubsystem>())
	{
		MenuSys->ShowMainMenu();
	}
}
