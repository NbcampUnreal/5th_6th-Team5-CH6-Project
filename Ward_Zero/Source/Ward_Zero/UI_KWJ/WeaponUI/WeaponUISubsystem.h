// WeaponUISubsystem.h
// 총기 UI 서브시스템 — StatusComp 델리게이트 기반

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

	/** 무기 교체 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void NotifyWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn);

	/** 무기 집어넣기 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void NotifyWeaponHolstered();

	/** 위젯 표시/숨기기 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void SetWeaponUIVisible(bool bVisible);

	/** StatusComp 델리게이트에 바인딩 (UIManager에서 호출) */
	void BindToStatusComponent(class UPlayerStatusComponent* StatusComp);

private:

	UPROPERTY()
	TSubclassOf<UWeaponStatusWidget> WidgetClass;

	UPROPERTY()
	UWeaponStatusWidget* WeaponWidget = nullptr;

	UWeaponStatusWidget* GetOrCreateWidget();

	/** OnPistolAmmoChanged 콜백 */
	UFUNCTION()
	void OnPistolAmmoChanged(int32 Current, int32 Max, int32 Reserve);

	/** OnSMGAmmoChanged 콜백 */
	UFUNCTION()
	void OnSMGAmmoChanged(int32 Current, int32 Max, int32 Reserve);

	/** 현재 무기 인덱스 (1=Pistol, 2=SMG) */
	int32 WeaponIdx = 1;

	/** 현재 바인딩된 StatusComp (재바인딩 시 이전 것 해제용) */
	UPROPERTY()
	class UPlayerStatusComponent* BoundStatusComp = nullptr;
};
