

#include "UI_KWJ/MainMenu/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Ward_Zero.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Start)
	{
		BTN_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}
	if (BTN_Settings)
	{
		BTN_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}
	if (BTN_Quit)
	{
		BTN_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	if (Anim_MenuIn)
	{
		PlayAnimation(Anim_MenuIn);
	}
}


void UMainMenuWidget::OnStartClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 시작 클릭"));

	if (Anim_MenuOut)
	{
		PlayAnimation(Anim_MenuOut);


		float AnimLength = Anim_MenuOut->GetEndTime();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				HideMenuAndPlay();
			}, AnimLength, false);
	}
	else
	{
		HideMenuAndPlay();
	}
}

void UMainMenuWidget::HideMenuAndPlay()
{

	SetVisibility(ESlateVisibility::Collapsed);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}

	UGameplayStatics::OpenLevel(this, FName("/Game/Level/Maps/L_WardZero"));
}


void UMainMenuWidget::OnSettingsClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 설정 클릭 (미구현)"));

}


void UMainMenuWidget::OnQuitClicked()
{
	UE_LOG(LogWard_Zero, Log, TEXT("메인메뉴: 종료 클릭"));

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(
			GetWorld(),
			PC,
			EQuitPreference::Quit,
			false
		);
	}
}