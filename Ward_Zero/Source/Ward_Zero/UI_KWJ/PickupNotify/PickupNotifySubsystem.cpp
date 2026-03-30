// PickupNotifySubsystem.cpp

#include "UI_KWJ/PickupNotify/PickupNotifySubsystem.h"
#include "UI_KWJ/PickupNotify/PickupNotifyWidget.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

// ════════════════════════════════════════════════════════
//  StatusComp 바인딩
// ════════════════════════════════════════════════════════

void UPickupNotifySubsystem::BindToStatusComponent(UPlayerStatusComponent* StatusComp)
{
	if (!StatusComp) return;

	// 이전 바인딩 해제
	if (BoundStatusComp && BoundStatusComp != StatusComp)
	{
		BoundStatusComp->OnHealingItemCountChanged.RemoveDynamic(
			this, &UPickupNotifySubsystem::OnHealingItemCountChanged);
		BoundStatusComp->OnPistolAmmoChanged.RemoveDynamic(
			this, &UPickupNotifySubsystem::OnPistolAmmoChanged);
		BoundStatusComp->OnSMGAmmoChanged.RemoveDynamic(
			this, &UPickupNotifySubsystem::OnSMGAmmoChanged);
	}

	BoundStatusComp = StatusComp;

	// 초기 기준값 설정 (바인딩 직후 증가 오탐 방지)
	PrevHealCount     = StatusComp->HealingItemCount;
	PrevPistolReserve = 0; // Reserve는 StatusComp에서 직접 접근 불가 → 0으로 초기화
	PrevSMGReserve    = 0;

	StatusComp->OnHealingItemCountChanged.AddDynamic(
		this, &UPickupNotifySubsystem::OnHealingItemCountChanged);
	StatusComp->OnPistolAmmoChanged.AddDynamic(
		this, &UPickupNotifySubsystem::OnPistolAmmoChanged);
	StatusComp->OnSMGAmmoChanged.AddDynamic(
		this, &UPickupNotifySubsystem::OnSMGAmmoChanged);

	UE_LOG(LogWard_Zero, Log, TEXT("PickupNotifySubsystem: StatusComp 바인딩 완료"));
}

// ════════════════════════════════════════════════════════
//  델리게이트 콜백
// ════════════════════════════════════════════════════════

void UPickupNotifySubsystem::OnHealingItemCountChanged(int32 NewCount)
{
	int32 Delta = NewCount - PrevHealCount;
	if (Delta > 0)
	{
		FString Msg = FString::Printf(TEXT("힐템 획득 +%d"), Delta);
		ShowPickup(FText::FromString(Msg));
	}
	PrevHealCount = NewCount;
}

void UPickupNotifySubsystem::OnPistolAmmoChanged(int32 Current, int32 Max, int32 Reserve)
{
	int32 Delta = Reserve - PrevPistolReserve;
	if (Delta > 0)
	{
		FString Msg = FString::Printf(TEXT("권총 탄 획득 +%d"), Delta);
		ShowPickup(FText::FromString(Msg));
	}
	PrevPistolReserve = Reserve;
}

void UPickupNotifySubsystem::OnSMGAmmoChanged(int32 Current, int32 Max, int32 Reserve)
{
	int32 Delta = Reserve - PrevSMGReserve;
	if (Delta > 0)
	{
		FString Msg = FString::Printf(TEXT("SMG 탄 획득 +%d"), Delta);
		ShowPickup(FText::FromString(Msg));
	}
	PrevSMGReserve = Reserve;
}

// ════════════════════════════════════════════════════════
//  픽업 알림 표시
// ════════════════════════════════════════════════════════

void UPickupNotifySubsystem::ShowPickup(const FText& PickupText)
{
	UPickupNotifyWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
		Widget->AddEntry(PickupText);
	}
}

// ════════════════════════════════════════════════════════
//  위젯 생성
// ════════════════════════════════════════════════════════

UPickupNotifyWidget* UPickupNotifySubsystem::GetOrCreateWidget()
{
	if (IsValid(NotifyWidget))
	{
		if (!NotifyWidget->IsInViewport())
		{
			NotifyWidget->AddToViewport(90);
			NotifyWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		return NotifyWidget;
	}

	NotifyWidget = nullptr;

	if (!NotifyWidgetClass)
	{
		NotifyWidgetClass = LoadClass<UPickupNotifyWidget>(
			nullptr,
			TEXT("/Game/UI/pickup/WBP_PickupNotify.WBP_PickupNotify_C")
		);
	}

	if (!NotifyWidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_PickupNotify를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	NotifyWidget = CreateWidget<UPickupNotifyWidget>(PC, NotifyWidgetClass);
	if (NotifyWidget)
	{
		NotifyWidget->AddToViewport(90);
		NotifyWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	return NotifyWidget;
}
