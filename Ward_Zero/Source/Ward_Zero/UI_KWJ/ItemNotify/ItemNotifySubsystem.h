// ItemNotifySubsystem.h
// 아이템 습득 알림 관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ItemNotifySubsystem.generated.h"

class UItemNotifyWidget;

UCLASS()
class WARD_ZERO_API UItemNotifySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** 아이템 습득 알림 표시 */
	UFUNCTION(BlueprintCallable, Category = "ItemNotify")
	void ShowItemNotify(const FText& ItemName, UTexture2D* ItemImage, const FText& KeyHint);

	/** 알림 숨기기 */
	UFUNCTION(BlueprintCallable, Category = "ItemNotify")
	void HideItemNotify();

private:

	UPROPERTY()
	TSubclassOf<UItemNotifyWidget> WidgetClass;

	UPROPERTY()
	UItemNotifyWidget* NotifyWidget = nullptr;

	UItemNotifyWidget* GetOrCreateWidget();
};
