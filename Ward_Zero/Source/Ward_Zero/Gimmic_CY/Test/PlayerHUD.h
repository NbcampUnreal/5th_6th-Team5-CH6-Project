#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

class UBorder;
class UTextBlock;
class UButton;
class ADoor;
class APlayerController;

UCLASS()
class WARD_ZERO_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void ShowPasscode(bool bShow) const;

	UFUNCTION()
	void PasscodeNubmer(int32 number);

	UFUNCTION()
	void ClickedButtonOne();

	UFUNCTION()
	void ClickedButtonTwo();

	UFUNCTION()
	void ClickedButtonThree();

	UFUNCTION()
	void ClickedButtonFour();

	UFUNCTION()
	void ClickedButtonFive();

	UFUNCTION()
	void ClickedButtonSix();

	UFUNCTION()
	void ClickedButtonSeven();

	UFUNCTION()
	void ClickedButtonEight();

	UFUNCTION()
	void ClickedButtonNine();

	UFUNCTION()
	void ClickedButtonZero();

	UFUNCTION()
	void ClickedButtonClear();

	UFUNCTION()
	void ClickedButtonEnter();

	UFUNCTION()
	void ClickedButtonExit();

	UFUNCTION()
	void ClearPasscodeEntries();

	UFUNCTION()
	void SetPasscode(int32 DoorPasscode, AActor* OverlappedDoor, APlayerController* PlayerController);


private:

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UBorder> PasscodeBorder;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UTextBlock> PasscodeTextOne;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UTextBlock> PasscodeTextTwo;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UTextBlock> PasscodeTextThree;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UTextBlock> PasscodeTextFour;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonOne;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonTwo;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonThree;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonFour;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonFive;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonSix;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonSeven;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonEight;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonNine;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonZero;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonClear;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonEnter;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> ButtonExit;

	UPROPERTY()
	int32 PasscodeCurrentIndex = 0;

	UPROPERTY()
	int32 PasscodeDigits = 4;

	UPROPERTY()
	int32 EnterPasscode;

	UPROPERTY()
	int32 PasscodeDoor;

	UPROPERTY()
	TArray<UTextBlock*> PasscodeText;

	UPROPERTY()
	TObjectPtr<AActor> Door;

	UPROPERTY()
	TObjectPtr<APlayerController> PC;
};
