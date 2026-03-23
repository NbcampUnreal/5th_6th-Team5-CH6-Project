// SplashWidget.cpp

#include "UI_KWJ/Splash/SplashWidget.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Ward_Zero.h"

void USplashWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 전부 투명
	if (IMG_Logo) IMG_Logo->SetRenderOpacity(0.0f);
	if (IMG_Background) IMG_Background->SetRenderOpacity(1.0f); // 검정 배경은 불투명

	SetIsFocusable(true);
}

void USplashWidget::PlaySplash()
{
	if (bIsPlaying) return;
	bIsPlaying = true;
	bFinished = false;

	SetVisibility(ESlateVisibility::Visible);
	SetKeyboardFocus();

	StartFadeIn();
}

// ════════════════════════════════════════════════════════
//  페이드 단계
// ════════════════════════════════════════════════════════

void USplashWidget::StartFadeIn()
{
	if (!IMG_Logo) { FinishSplash(); return; }

	IMG_Logo->SetRenderOpacity(0.0f);

	// FadeInDuration 동안 0→1 보간
	float Elapsed = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FadeTimerHandle, [this, Elapsed]() mutable
		{
			Elapsed += 0.016f; // ~60Hz
			float Alpha = FMath::Clamp(Elapsed / FadeInDuration, 0.0f, 1.0f);

			if (IMG_Logo) IMG_Logo->SetRenderOpacity(Alpha);

			if (Alpha >= 1.0f)
			{
				if (UWorld* W = GetWorld())
				{
					W->GetTimerManager().ClearTimer(FadeTimerHandle);
				}
				StartHold();
			}
		}, 0.016f, true);
	}
}

void USplashWidget::StartHold()
{
	// HoldDuration 동안 유지 후 페이드아웃
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FadeTimerHandle, [this]()
		{
			StartFadeOut();
		}, HoldDuration, false);
	}
}

void USplashWidget::StartFadeOut()
{
	if (!IMG_Logo) { FinishSplash(); return; }

	float Elapsed = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FadeTimerHandle, [this, Elapsed]() mutable
		{
			Elapsed += 0.016f;
			float Alpha = FMath::Clamp(1.0f - (Elapsed / FadeOutDuration), 0.0f, 1.0f);

			if (IMG_Logo) IMG_Logo->SetRenderOpacity(Alpha);

			if (Alpha <= 0.0f)
			{
				if (UWorld* W = GetWorld())
				{
					W->GetTimerManager().ClearTimer(FadeTimerHandle);
				}
				FinishSplash();
			}
		}, 0.016f, true);
	}
}

void USplashWidget::FinishSplash()
{
	if (bFinished) return;
	bFinished = true;
	bIsPlaying = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}

	SetVisibility(ESlateVisibility::Collapsed);
	OnSplashFinished.Broadcast();

	UE_LOG(LogWard_Zero, Log, TEXT("스플래시 완료"));
}

// ════════════════════════════════════════════════════════
//  스킵 (클릭 또는 아무 키)
// ════════════════════════════════════════════════════════

void USplashWidget::SkipSplash()
{
	if (!bIsPlaying || bFinished) return;

	UE_LOG(LogWard_Zero, Log, TEXT("스플래시 스킵"));
	FinishSplash();
}

FReply USplashWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	SkipSplash();
	return FReply::Handled();
}

FReply USplashWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	SkipSplash();
	return FReply::Handled();
}
