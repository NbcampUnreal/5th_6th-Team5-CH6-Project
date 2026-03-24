#include "Gimmic_CY/Test/PlayerHUD.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Ward_ZeroPlayerController.h"

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	UE_LOG(LogTemp, Warning, TEXT("=== UI CHECK START ==="));

	UE_LOG(LogTemp, Warning, TEXT("Border: %s"), PasscodeBorder ? TEXT("OK") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("Text1: %s"), PasscodeTextOne ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Text2: %s"), PasscodeTextTwo ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Text3: %s"), PasscodeTextThree ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Text4: %s"), PasscodeTextFour ? TEXT("OK") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("Btn1: %s"), ButtonOne ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn2: %s"), ButtonTwo ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn3: %s"), ButtonThree ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn4: %s"), ButtonFour ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn5: %s"), ButtonFive ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn6: %s"), ButtonSix ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn7: %s"), ButtonSeven ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn8: %s"), ButtonEight ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn9: %s"), ButtonNine ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Btn0: %s"), ButtonZero ? TEXT("OK") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("Clear: %s"), ButtonClear ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Enter: %s"), ButtonEnter ? TEXT("OK") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Exit: %s"), ButtonExit ? TEXT("OK") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("=== UI CHECK END ==="));
	
	if (!PasscodeBorder) return;

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

	PasscodeText = {
	   PasscodeTextOne,
	   PasscodeTextTwo,
	   PasscodeTextThree,
	   PasscodeTextFour
	};
}

void UPlayerHUD::ShowPasscode(bool bShow) const
{
	if (bShow)
	{
		if (PasscodeBorder)
		{
			PasscodeBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PasscodeBorder is NULL"));
		}
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
		EnterPasscode = EnterPasscode * 10 + number;

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
			UE_LOG(LogTemp, Warning, TEXT("Collect"));
			//Door->bDoorPasscode = true;
			//Door->PasscodeCorrect();
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
			ShowPasscode(false);
			ClearPasscodeEntries();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed"));
		}

	}
}

void UPlayerHUD::ClickedButtonExit()
{
	ShowPasscode(false);
	//ShowInteract(true);
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
	EnterPasscode = 0;
}

void UPlayerHUD::SetPasscode(int32 DoorPasscode, AActor* OverlappedDoor, APlayerController* PlayerController)
{
	PasscodeDoor = DoorPasscode;
	Door = OverlappedDoor;
	PC = PlayerController;
}
