#include "Weapon/WZ_HUD_DH.h"
#include "Blueprint/UserWidget.h"

void AWZ_HUD_DH::BeginPlay()
{
	Super::BeginPlay();

	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);

		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();

			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AWZ_HUD_DH::SetCrosshairVisibility(bool bIsVisible)
{
	if (CrosshairWidget)
	{
		if (bIsVisible)
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
