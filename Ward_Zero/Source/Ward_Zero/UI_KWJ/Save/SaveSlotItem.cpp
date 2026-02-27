// SaveSlotItem.cpp

#include "UI_KWJ/Save/SaveSlotItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USaveSlotItem::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Slot)
	{
		BTN_Slot->OnClicked.AddDynamic(this, &USaveSlotItem::OnButtonClicked);
	}
}

void USaveSlotItem::SetSlotInfo(const FSaveFileInfo& InInfo)
{
	SlotInfo = InInfo;

	if (TXT_SlotLabel)
	{
		FString TimeStr = InInfo.SaveDateTime.ToString(TEXT("%Y/%m/%d %H:%M"));
		TXT_SlotLabel->SetText(FText::FromString(
			FString::Printf(TEXT("%s  |  %s  |  %s"),
				*InInfo.DisplayName, *TimeStr, *InInfo.LevelName.ToString())
		));
	}
}

void USaveSlotItem::OnButtonClicked()
{
	OnClicked_SlotItem.ExecuteIfBound(SlotInfo);
}
