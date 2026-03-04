#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameClearSubsystem.generated.h"

class UGameClearWidget;

UCLASS()
class WARD_ZERO_API UGameClearSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "GameClear")
	void ShowGameClear(float PlayTimeSeconds);

	UFUNCTION(BlueprintCallable, Category = "GameClear")
	void HideGameClear();

	UFUNCTION(BlueprintPure, Category = "GameClear")
	bool IsGameClearShowing() const;

protected:

	UPROPERTY()
	TSubclassOf<UGameClearWidget> GameClearWidgetClass;

	UPROPERTY()
	UGameClearWidget* GameClearWidget;

	UGameClearWidget* GetOrCreateWidget();
};
