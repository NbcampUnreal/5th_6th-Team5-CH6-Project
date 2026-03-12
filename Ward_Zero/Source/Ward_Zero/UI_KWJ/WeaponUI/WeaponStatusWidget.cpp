// Fill out your copyright notice in the Description page of Project Settings.
// WeaponStatusWidget.cpp

#include "UI_KWJ/WeaponUI/WeaponStatusWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Ward_Zero.h"

void UWeaponStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PistolCurrentScale = ActiveScale;
	PistolTargetScale = ActiveScale;
	SMGCurrentScale = ActiveScale;
	SMGTargetScale = ActiveScale;

	// 초기에는 둘 다 숨김 (무기를 꺼내야 보임)
	if (IMG_Pistol) IMG_Pistol->SetVisibility(ESlateVisibility::Collapsed);
	if (IMG_SMG) IMG_SMG->SetVisibility(ESlateVisibility::Collapsed);
}

void UWeaponStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 스케일 보간
	PistolCurrentScale = FMath::FInterpTo(PistolCurrentScale, PistolTargetScale, InDeltaTime, ScaleInterpSpeed);
	SMGCurrentScale = FMath::FInterpTo(SMGCurrentScale, SMGTargetScale, InDeltaTime, ScaleInterpSpeed);

	if (IMG_Pistol && IMG_Pistol->GetVisibility() != ESlateVisibility::Collapsed)
	{
		ApplyImageScale(IMG_Pistol, PistolCurrentScale);
	}
	if (IMG_SMG && IMG_SMG->GetVisibility() != ESlateVisibility::Collapsed)
	{
		ApplyImageScale(IMG_SMG, SMGCurrentScale);
	}
}

// ════════════════════════════════════════════════════════
//  무기 교체
// ════════════════════════════════════════════════════════

void UWeaponStatusWidget::OnWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn)
{
	ActiveWeaponIndex = NewWeaponIndex;
	bWeaponDrawn = bIsDrawn;

	if (!bWeaponDrawn)
	{
		OnWeaponHolstered();
		return;
	}

	// 활성 무기만 보이게, 비활성 무기는 완전히 숨김
	ApplyVisibility();

	// 활성 무기 하이라이트 (크기 확대)
	if (ActiveWeaponIndex == 1)
	{
		PistolTargetScale = HighlightScale;
		PistolCurrentScale = ActiveScale; // 즉시 기본 크기에서 시작 → 하이라이트로 보간
	}
	else if (ActiveWeaponIndex == 2)
	{
		SMGTargetScale = HighlightScale;
		SMGCurrentScale = ActiveScale;
	}

	// 하이라이트 타이머 리셋 & 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HighlightTimerHandle);
		World->GetTimerManager().SetTimer(
			HighlightTimerHandle,
			this,
			&UWeaponStatusWidget::EndHighlight,
			HighlightDuration,
			false
		);
	}

	UE_LOG(LogWard_Zero, Log, TEXT("WeaponUI: 무기 교체 인덱스 %d"), NewWeaponIndex);
}

void UWeaponStatusWidget::OnWeaponHolstered()
{
	bWeaponDrawn = false;
	ActiveWeaponIndex = 0;

	// 무기를 집어넣으면 둘 다 숨김
	if (IMG_Pistol) IMG_Pistol->SetVisibility(ESlateVisibility::Collapsed);
	if (IMG_SMG) IMG_SMG->SetVisibility(ESlateVisibility::Collapsed);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HighlightTimerHandle);
	}
}

void UWeaponStatusWidget::EndHighlight()
{
	// 하이라이트 종료 → 기본 스케일로 복귀
	if (ActiveWeaponIndex == 1)
	{
		PistolTargetScale = ActiveScale;
	}
	else if (ActiveWeaponIndex == 2)
	{
		SMGTargetScale = ActiveScale;
	}
}

// ════════════════════════════════════════════════════════
//  탄약 표시
// ════════════════════════════════════════════════════════

void UWeaponStatusWidget::UpdateAmmoDisplay(int32 CurrentAmmo, int32 MaxCapacity, int32 ReserveAmmo)
{
	if (TXT_CurrentAmmo)
	{
		// 현재 탄약 / 전체 보유 탄약
		FString AmmoStr = FString::Printf(TEXT("%d / %d"), CurrentAmmo, ReserveAmmo);
		TXT_CurrentAmmo->SetText(FText::FromString(AmmoStr));
	}
}

// ════════════════════════════════════════════════════════
//  표시/숨기기
// ══════════════════════════════════════════════════���═════

void UWeaponStatusWidget::SetWeaponUIVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

// ════════════════════════════════════════════════════════
//  내부: 가시성 제어
// ════════════════════════════════════════════════════════

void UWeaponStatusWidget::ApplyVisibility()
{
	if (!bWeaponDrawn)
	{
		if (IMG_Pistol) IMG_Pistol->SetVisibility(ESlateVisibility::Collapsed);
		if (IMG_SMG) IMG_SMG->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 활성 무기만 보이기
	if (ActiveWeaponIndex == 1)
	{
		if (IMG_Pistol) IMG_Pistol->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (IMG_SMG) IMG_SMG->SetVisibility(ESlateVisibility::Collapsed);
	}
	else if (ActiveWeaponIndex == 2)
	{
		if (IMG_Pistol) IMG_Pistol->SetVisibility(ESlateVisibility::Collapsed);
		if (IMG_SMG) IMG_SMG->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UWeaponStatusWidget::ApplyImageScale(UImage* InImage, float Scale)
{
	if (!InImage) return;

	FWidgetTransform Transform;
	Transform.Scale = FVector2D(Scale, Scale);
	InImage->SetRenderTransform(Transform);
}