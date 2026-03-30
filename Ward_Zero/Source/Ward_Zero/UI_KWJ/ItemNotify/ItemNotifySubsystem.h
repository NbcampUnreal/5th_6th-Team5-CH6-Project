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

	/**
	 * 아이템 인덱스로 습득 알림 표시
	 * DataTable 조회 + 최초 습득 체크 + 위젯 표시 + MarkItemNotified 전부 내부 처리
	 * 아이템 액터에서는 이것만 호출하면 됨
	 */
	UFUNCTION(BlueprintCallable, Category = "ItemNotify")
	void ShowItemNotifyByIndex(int32 DocIdx);

	/** 아이템 습득 알림 표시 (수동 — 직접 파라미터 지정) */
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
