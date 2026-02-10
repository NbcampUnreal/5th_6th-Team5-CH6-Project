// MainMenuSubsystem.h
// 메인 메뉴 위젯 생성/관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MainMenuSubsystem.generated.h"

class UMainMenuWidget;

UCLASS()
class UMainMenuSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** 메인 메뉴 표시 */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowMainMenu();

	/** 메인 메뉴 숨기기 */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void HideMainMenu();

	/** 메인 메뉴가 열려있는지 */
	UFUNCTION(BlueprintPure, Category = "MainMenu")
	bool IsMainMenuOpen() const;

protected:

	UPROPERTY()
	TSubclassOf<UMainMenuWidget> MenuWidgetClass;

	UPROPERTY()
	UMainMenuWidget* MenuWidget;

	UMainMenuWidget* GetOrCreateMenu();
};
