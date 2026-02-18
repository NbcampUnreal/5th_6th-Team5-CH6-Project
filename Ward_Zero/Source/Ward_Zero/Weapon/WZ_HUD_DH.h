// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WZ_HUD_DH.generated.h"

UCLASS()
class WARD_ZERO_API AWZ_HUD_DH : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> CrosshairWidgetClass;

	void SetCrosshairVisibility(bool bIsVisible);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class UUserWidget* CrosshairWidget;
};
