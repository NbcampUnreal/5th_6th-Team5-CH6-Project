// PickupNotifySubsystem.h
// 아이템 픽업 알림 서브시스템
// — StatusComp 델리게이트 감지 → 증가 시 픽업 알림 표시

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PickupNotifySubsystem.generated.h"

class UPickupNotifyWidget;
class UPlayerStatusComponent;

UCLASS()
class WARD_ZERO_API UPickupNotifySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** StatusComp 델리게이트에 바인딩 (UIManager에서 호출) */
	void BindToStatusComponent(UPlayerStatusComponent* StatusComp);

private:

	// ── 델리게이트 콜백 ──

	UFUNCTION()
	void OnHealingItemCountChanged(int32 NewCount);

	UFUNCTION()
	void OnPistolAmmoChanged(int32 Current, int32 Max, int32 Reserve);

	UFUNCTION()
	void OnSMGAmmoChanged(int32 Current, int32 Max, int32 Reserve);

	// ── 이전 값 추적 (증가 감지용) ──

	int32 PrevHealCount    = 0;
	int32 PrevPistolReserve = 0;
	int32 PrevSMGReserve   = 0;

	// ── 위젯 ──

	void ShowPickup(const FText& PickupText);

	UPickupNotifyWidget* GetOrCreateWidget();

	UPROPERTY()
	UPickupNotifyWidget* NotifyWidget = nullptr;

	UPROPERTY()
	TSubclassOf<UPickupNotifyWidget> NotifyWidgetClass;

	// ── 재바인딩 방지 ──

	UPROPERTY()
	UPlayerStatusComponent* BoundStatusComp = nullptr;
};
