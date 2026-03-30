// PickupNotifySubsystem.h
// 아이템 픽업 알림 서브시스템
// — 아이템 액터가 습득 시 ShowPickup() 직접 호출

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PickupNotifySubsystem.generated.h"

class UPickupNotifyWidget;

UCLASS()
class WARD_ZERO_API UPickupNotifySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * 픽업 알림 표시 (아이템 액터에서 직접 호출)
	 * @param PickupText 표시 텍스트 (예: "치료제 +1", "권총 탄 +15", "카드키")
	 */
	UFUNCTION(BlueprintCallable, Category = "PickupNotify")
	void ShowPickup(const FText& PickupText);

private:

	UPickupNotifyWidget* GetOrCreateWidget();

	UPROPERTY()
	UPickupNotifyWidget* NotifyWidget = nullptr;

	UPROPERTY()
	TSubclassOf<UPickupNotifyWidget> NotifyWidgetClass;
};
