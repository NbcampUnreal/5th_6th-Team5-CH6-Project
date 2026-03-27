// InteractionHintWidget.cpp

#include "UI_KWJ/InteractionHint/InteractionHintWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UInteractionHintWidget::ShowMessage(float Duration)
{
    SetVisibility(ESlateVisibility::HitTestInvisible);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
        World->GetTimerManager().SetTimer(AutoHideTimerHandle, [this]()
            {
                HideMessage();
            }, Duration, false);
    }
}

void UInteractionHintWidget::ShowMessageWithText(const FText& Message, float Duration)
{
    if (TXT_Message)
    {
        TXT_Message->SetText(Message);
    }
    ShowMessage(Duration);
}
void UInteractionHintWidget::HideMessage()
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoHideTimerHandle);
	}
}
