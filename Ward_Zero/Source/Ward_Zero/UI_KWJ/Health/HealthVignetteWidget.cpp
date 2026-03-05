// HealthVignetteWidget.cpp

#include "UI_KWJ/Health/HealthVignetteWidget.h"
#include "Components/Image.h"
#include "Ward_Zero.h"

void UHealthVignetteWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 완전 투명
	if (IMG_BloodVignette)
	{
		IMG_BloodVignette->SetRenderOpacity(0.0f);
	}

	CurrentOpacity = 0.0f;
	TargetOpacity = 0.0f;
}

void UHealthVignetteWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 부드러운 전환
	if (!FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.001f))
	{
		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, InDeltaTime, InterpSpeed);

		if (IMG_BloodVignette)
		{
			IMG_BloodVignette->SetRenderOpacity(CurrentOpacity);
		}
	}
}

void UHealthVignetteWidget::UpdateVignette(float HealthPercent)
{
	// 체력 비율 클램핑
	HealthPercent = FMath::Clamp(HealthPercent, 0.0f, 1.0f);

	if (HealthPercent >= VignetteStartThreshold)
	{
		// 체력이 임계값 이상이면 비네팅 없음
		TargetOpacity = 0.0f;
	}
	else
	{
		// 임계값 이하: 체력이 낮을수록 비네팅 강해짐
		// 0.75 → 0.0,  0.0 → MaxVignetteOpacity
		float Ratio = 1.0f - (HealthPercent / VignetteStartThreshold);

		// 비선형 커브 — 체력이 낮을수록 급격히 강해짐
		Ratio = FMath::Pow(Ratio, 1.5f);

		TargetOpacity = Ratio * MaxVignetteOpacity;
	}

	UE_LOG(LogWard_Zero, Verbose, TEXT("HealthVignette: HP %.0f%% → Opacity %.2f"),
		HealthPercent * 100.0f, TargetOpacity);
}

void UHealthVignetteWidget::OnHealthChanged(float CurrentHP, float MaxHP)
{
	if (MaxHP <= 0.0f) return;

	float Percent = CurrentHP / MaxHP;
	UpdateVignette(Percent);
}
