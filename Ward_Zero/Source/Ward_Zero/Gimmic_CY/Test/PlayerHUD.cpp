#include "Gimmic_CY/Test/PlayerHUD.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Ward_ZeroPlayerController.h"

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	InteractBorder->SetVisibility(ESlateVisibility::Hidden);
	PasscodeBorder->SetVisibility(ESlateVisibility::Hidden);
	ButtonOne->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonOne);
	ButtonTwo->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonTwo);
	ButtonThree->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonThree);
	ButtonFour->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonFour);
	ButtonFive->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonFive);
	ButtonSix->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonSix);
	ButtonSeven->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonSeven);
	ButtonEight->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonEight);
	ButtonNine->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonNine);
	ButtonZero->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonZero);
	ButtonClear->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonClear);
	ButtonEnter->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonEnter);
	ButtonExit->OnClicked.AddDynamic(this, &UPlayerHUD::ClickedButtonExit);
}

void UPlayerHUD::ShowInteract(bool bShow) const
{
	if (bShow)
	{
		InteractBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerHUD::ShowPasscode(bool bShow) const
{
	if (bShow)
	{
		PasscodeBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		PasscodeBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerHUD::PasscodeNubmer(int32 number)
{
	if (PasscodeCurrentIndex < PasscodeDigits)
	{
		FString CodeString = FString::FromInt(EnterPasscode);
		FString NumberString = FString::FromInt(number);
		FString AppendedCode = CodeString.Append(NumberString);

		EnterPasscode = FCString::Atoi(*AppendedCode);

		PasscodeText.Add(PasscodeTextOne);
		PasscodeText.Add(PasscodeTextTwo);
		PasscodeText.Add(PasscodeTextThree);
		PasscodeText.Add(PasscodeTextFour);

		if (PasscodeText.IsValidIndex(PasscodeCurrentIndex))
		{
			UTextBlock* currentText = PasscodeText[PasscodeCurrentIndex];
			currentText->SetVisibility(ESlateVisibility::Visible);
			currentText->SetText(FText::AsNumber(number));
			PasscodeCurrentIndex++;
		}
	}
}

void UPlayerHUD::ClickedButtonOne() { PasscodeNubmer(1); }

void UPlayerHUD::ClickedButtonTwo() { PasscodeNubmer(2); }

void UPlayerHUD::ClickedButtonThree() { PasscodeNubmer(3); }

void UPlayerHUD::ClickedButtonFour() { PasscodeNubmer(4); }

void UPlayerHUD::ClickedButtonFive() { PasscodeNubmer(5); }

void UPlayerHUD::ClickedButtonSix() { PasscodeNubmer(6); }

void UPlayerHUD::ClickedButtonSeven() { PasscodeNubmer(7); }

void UPlayerHUD::ClickedButtonEight() { PasscodeNubmer(8); }

void UPlayerHUD::ClickedButtonNine() { PasscodeNubmer(9); }

void UPlayerHUD::ClickedButtonZero() { PasscodeNubmer(0); }

void UPlayerHUD::ClickedButtonClear(){ ClearPasscodeEntries(); }

void UPlayerHUD::ClickedButtonEnter()
{
	if (PasscodeCurrentIndex == PasscodeDigits)
	{
		if (EnterPasscode == PasscodeDoor)
		{
			//Door->bDoorPasscode = true;
			//Door->PasscodeCorrect();
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
			ShowPasscode(false);
			ClearPasscodeEntries();
		}
	}
}

void UPlayerHUD::ClickedButtonExit()
{
	ShowPasscode(false);
	ShowInteract(true);
	ClearPasscodeEntries();
	PC->SetShowMouseCursor(false);
	PC->SetInputMode(FInputModeGameOnly());
}

void UPlayerHUD::ClearPasscodeEntries()
{
	for (size_t i = 0; i < PasscodeCurrentIndex; i++)
	{
		UTextBlock* currentText = PasscodeText[i];
		currentText->SetVisibility(ESlateVisibility::Visible);
		currentText->SetText(FText::AsNumber(0));
	}
	PasscodeCurrentIndex = 0;
	EnterPasscode = NULL;
}

void UPlayerHUD::SetPasscode(int32 DoorPasscode, ADoor* OverlappedDoor, AWard_ZeroPlayerController* PlayerController)
{
	PasscodeDoor = DoorPasscode;
	Door = OverlappedDoor;
	PC = PlayerController;
}
