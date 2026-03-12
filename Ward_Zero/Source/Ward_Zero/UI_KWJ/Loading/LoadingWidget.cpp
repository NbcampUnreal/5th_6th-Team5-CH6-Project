// LoadingWidget.cpp

#include "UI_KWJ/Loading/LoadingWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Throbber.h"

void ULoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULoadingWidget::SetLoadingMessage(const FText& Message)
{
	if (TXT_Loading)
	{
		TXT_Loading->SetText(Message);
	}
}
