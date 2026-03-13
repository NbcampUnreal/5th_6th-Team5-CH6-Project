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

void UHealthVignetteWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InterpTimerHandle);
	}
	Super::NativeDestruct();
}

void UHealthVignetteWidget::UpdateOpacityInterp()
{
	if (FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.001f))
	{
		CurrentOpacity = TargetOpacity;
		if (IMG_BloodVignette)
		{
			IMG_BloodVignette->SetRenderOpacity(CurrentOpacity);
		}
		// 보간 완료 → 타이머 정지
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InterpTimerHandle);
		}
		return;
	}

	// 30Hz 기준 DeltaTime ≈ 0.033
	CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, 0.033f, InterpSpeed);

	if (IMG_BloodVignette)
	{
		IMG_BloodVignette->SetRenderOpacity(CurrentOpacity);
	}
}

void UHealthVignetteWidget::UpdateVignette(float HealthPercent)
{
	// 체력 비율 클램핑
	HealthPercent = FMath::Clamp(HealthPercent, 0.0f, 1.0f);

	if (HealthPercent >= VignetteStartThreshold)
	{
		TargetOpacity = 0.0f;
	}
	else
	{
		float Ratio = 1.0f - (HealthPercent / VignetteStartThreshold);
		Ratio = FMath::Pow(Ratio, 1.5f);
		TargetOpacity = Ratio * MaxVignetteOpacity;
	}

	// 보간이 필요하면 타이머 시작 (30Hz)
	if (!FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.001f))
	{
		if (UWorld* World = GetWorld())
		{
			if (!World->GetTimerManager().IsTimerActive(InterpTimerHandle))
			{
				World->GetTimerManager().SetTimer(
					InterpTimerHandle, this,
					&UHealthVignetteWidget::UpdateOpacityInterp,
					0.033f, true); // 30Hz 반복
			}
		}
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
