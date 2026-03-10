// Fill out your copyright notice in the Description page of Project Settings.

// WeaponUISubsystem.h
// 총기 상태 UI 서브시스템
// 무기 교체 / 발사 / 재장전 시 위젯 갱신

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

	/** 매 틱 탄약 정보 갱신 (캐릭터 Tick에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void UpdateWeaponStatus();

	/** 무기 교체 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void NotifyWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn);

	/** 무기 집어넣기 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void NotifyWeaponHolstered();

	/** 위젯 표시/숨김 */
	UFUNCTION(BlueprintCallable, Category = "WeaponUI")
	void SetWeaponUIVisible(bool bVisible);

private:

	UPROPERTY()
	TSubclassOf<UWeaponStatusWidget> WidgetClass;

	UPROPERTY()
	UWeaponStatusWidget* WeaponWidget = nullptr;

	UWeaponStatusWidget* GetOrCreateWidget();
};