#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "GameClearWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UCanvasPanel;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UGameClearWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UGameClearWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		bIsFocusable = true;
	}

	UFUNCTION(BlueprintCallable, Category = "GameClear")
	void ShowResult(float InPlayTimeSeconds);

	void PlayFadeIn();

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_MainMenu;

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_Quit;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Title;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_PlayTime;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TXT_Credits;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IMG_Background;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* Panel_Buttons;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Anim_FadeIn;

protected:

	virtual void NativeConstruct() override;

private:

	FString FormatPlayTime(float TotalSeconds) const;

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnQuitClicked();
};
