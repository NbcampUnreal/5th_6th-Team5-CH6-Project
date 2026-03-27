// HealItemSubsystem.h
// 힐템 UI 관리 서브시스템

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "HealItemSubsystem.generated.h"

class UHealItemWidget;

UCLASS()
class WARD_ZERO_API UHealItemSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** 힐템 수량 갱신 (캐릭터/아이템에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "HealItem")
	void UpdateHealCount(int32 Current, int32 Max);

	/** 힐템 UI 표시/숨기기 */
	UFUNCTION(BlueprintCallable, Category = "HealItem")
	void SetHealUIVisible(bool bVisible);

private:

	UPROPERTY()
	TSubclassOf<UHealItemWidget> WidgetClass;

	UPROPERTY()
	UHealItemWidget* HealWidget = nullptr;

	UHealItemWidget* GetOrCreateWidget();
};
