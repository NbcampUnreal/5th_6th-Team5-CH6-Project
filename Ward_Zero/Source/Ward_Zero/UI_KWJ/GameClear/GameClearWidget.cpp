#include "UI_KWJ/GameClear/GameClearWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Ward_Zero.h"

void UGameClearWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_MainMenu)
		BTN_MainMenu->OnClicked.AddDynamic(this, &UGameClearWidget::OnMainMenuClicked);

	if (BTN_Quit)
		BTN_Quit->OnClicked.AddDynamic(this, &UGameClearWidget::OnQuitClicked);
}

void UGameClearWidget::ShowResult(float InPlayTimeSeconds)
{
	if (TXT_PlayTime)
	{
		FString TimeStr = FString::Printf(TEXT("클리어 타임   %s"), *FormatPlayTime(InPlayTimeSeconds));
		TXT_PlayTime->SetText(FText::FromString(TimeStr));
	}
	PlayFadeIn();
}

void UGameClearWidget::PlayFadeIn()
{
	if (Anim_FadeIn)
	{
		PlayAnimation(Anim_FadeIn);
		return;
	}
	if (IMG_Background) IMG_Background->SetRenderOpacity(0.85f);
	if (TXT_Title)      TXT_Title->SetRenderOpacity(1.0f);
	if (TXT_PlayTime)   TXT_PlayTime->SetRenderOpacity(1.0f);
	if (TXT_Credits)    TXT_Credits->SetRenderOpacity(1.0f);
	if (Panel_Buttons)  Panel_Buttons->SetRenderOpacity(1.0f);
}

FString UGameClearWidget::FormatPlayTime(float TotalSeconds) const
{
	int32 T = FMath::FloorToInt(TotalSeconds);
	return FString::Printf(TEXT("%02d:%02d:%02d"), T / 3600, (T % 3600) / 60, T % 60);
}

void UGameClearWidget::OnMainMenuClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameClear: 메인 메뉴로 이동"));
	SetVisibility(ESlateVisibility::Collapsed);
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("/Game/UI/Demo"), true);
	}
}

void UGameClearWidget::OnQuitClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("GameClear: 게임 종료"));
	if (APlayerController* PC = GetOwningPlayer())
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}