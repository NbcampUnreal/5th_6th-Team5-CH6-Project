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

	/** 힐템 수량 갱신 (캐릭터/아이템에서 직접 호출 가능) */
	UFUNCTION(BlueprintCallable, Category = "HealItem")
	void UpdateHealCount(int32 Current, int32 Max);

	/** 힐템 UI 표시/숨기기 */
	UFUNCTION(BlueprintCallable, Category = "HealItem")
	void SetHealUIVisible(bool bVisible);

	/** StatusComp 델리게이트에 바인딩 (UIManager에서 호출) */
	void BindToStatusComponent(class UPlayerStatusComponent* StatusComp);

private:

	/** OnHealingItemCountChanged 콜백 */
	UFUNCTION()
	void OnHealingItemCountChanged(int32 NewCount);

	/** Max 수량 캐시 (델리게이트가 현재 수량만 보내므로) */
	int32 CachedMaxCount = 5;

	/** 현재 바인딩된 StatusComp (재바인딩 시 이전 것 해제용) */
	UPROPERTY()
	class UPlayerStatusComponent* BoundStatusComp = nullptr;

	UPROPERTY()
	TSubclassOf<UHealItemWidget> WidgetClass;

	UPROPERTY()
	UHealItemWidget* HealWidget = nullptr;

	UHealItemWidget* GetOrCreateWidget();
};
