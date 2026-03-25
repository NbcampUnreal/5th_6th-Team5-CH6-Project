// InteractionHintSubsystem.cpp

#include "UI_KWJ/InteractionHint/InteractionHintSubsystem.h"
#include "UI_KWJ/InteractionHint/InteractionHintWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UInteractionHintSubsystem::ShowHint(float Duration)
{
	UInteractionHintWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->ShowMessage(Duration);
	}
}

void UInteractionHintSubsystem::HideHint()
{
	if (HintWidget)
	{
		HintWidget->HideMessage();
	}
}

UInteractionHintWidget* UInteractionHintSubsystem::GetOrCreateWidget()
{
	if (IsValid(HintWidget))
	{
		if (!HintWidget->IsInViewport())
		{
			HintWidget->AddToViewport(50);
			HintWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return HintWidget;
	}

	HintWidget = nullptr;

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UInteractionHintWidget>(
			nullptr,
			TEXT("/Game/UI/InteractionHint/WBP_InteractionHint.WBP_InteractionHint_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_InteractionHint를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	HintWidget = CreateWidget<UInteractionHintWidget>(PC, WidgetClass);
	if (HintWidget)
	{
		HintWidget->AddToViewport(50);
		HintWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return HintWidget;
}
