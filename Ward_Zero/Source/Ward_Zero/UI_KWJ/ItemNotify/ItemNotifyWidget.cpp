// ItemNotifyWidget.cpp

#include "UI_KWJ/ItemNotify/ItemNotifyWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "Ward_Zero.h"

UItemNotifyWidget::UItemNotifyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UItemNotifyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Close)
	{
		BTN_Close->OnClicked.AddDynamic(this, &UItemNotifyWidget::OnCloseClicked);
	}

	// 텍스트 자동 줄바꿈
	if (TXT_ItemName)
	{
		TXT_ItemName->SetAutoWrapText(true);
	}
	if (TXT_KeyHint)
	{
		TXT_KeyHint->SetAutoWrapText(true);
	}
}

void UItemNotifyWidget::ShowNotify(const FText& ItemName, UTexture2D* ItemImage, const FText& KeyHint)
{
	// 아이템 이름
	if (TXT_ItemName)
	{
		TXT_ItemName->SetText(ItemName);
	}

	// 아이템 이미지
	if (IMG_Item && ItemImage)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(ItemImage);
		Brush.ImageSize = FVector2D(128.f, 128.f);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		IMG_Item->SetBrush(Brush);
		IMG_Item->SetColorAndOpacity(FLinearColor::White);
	}

	// 사용 키 안내
	if (TXT_KeyHint)
	{
		TXT_KeyHint->SetText(KeyHint);
	}

	SetVisibility(ESlateVisibility::Visible);

	// 자동 닫기 타이머
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
		World->GetTimerManager().SetTimer(AutoCloseTimerHandle, [this]()
		{
			HideNotify();
		}, AutoCloseDuration, false);
	}

	UE_LOG(LogWard_Zero, Log, TEXT("아이템 알림: %s"), *ItemName.ToString());
}

void UItemNotifyWidget::HideNotify()
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}

	OnHiddenDelegate.ExecuteIfBound();
	OnHiddenDelegate.Unbind();
}

void UItemNotifyWidget::OnCloseClicked()
{
	HideNotify();
}
