// WeaponUISubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "WeaponUISubsystem.generated.h"

class UWeaponStatusWidget;

UCLASS()
class WARD_ZERO_API UWeaponUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 무기 교체 시 호출 — 탄약 폴링 시작 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void NotifyWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn);

	/** 무기 집어넣기 시 호출 — 탄약 폴링 중지 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void NotifyWeaponHolstered();

	/** 위젯 표시/숨기기 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void SetWeaponUIVisible(bool bVisible);

private:

	UPROPERTY()
	TSubclassOf<UWeaponStatusWidget> WidgetClass;

	UPROPERTY()
	UWeaponStatusWidget* WeaponWidget = nullptr;

	UWeaponStatusWidget* GetOrCreateWidget();

	/** 탄약 갱신 폴링 (10Hz — 캐릭터 Tick 대체) */
	FTimerHandle AmmoUpdateTimerHandle;
	void PollAmmoUpdate();
	void StartAmmoPolling();
	void StopAmmoPolling();
};
