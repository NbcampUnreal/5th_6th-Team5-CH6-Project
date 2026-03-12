// PauseMenuSubsystem.h
// ESC 일시정지 메뉴 — 불러오기, 옵션, 메인 메뉴

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PauseMenuSubsystem.generated.h"

class UPauseMenuWidget;

UCLASS()
class WARD_ZERO_API UPauseMenuSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** ESC 키에서 호출 — 열려있으면 닫고, 닫혀있으면 열기 */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void HidePauseMenu();

	UFUNCTION(BlueprintPure, Category = "PauseMenu")
	bool IsPauseMenuOpen() const;

private:

	UPROPERTY()
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY()
	UPauseMenuWidget* PauseMenuWidget;

	UPauseMenuWidget* GetOrCreateWidget();
};
