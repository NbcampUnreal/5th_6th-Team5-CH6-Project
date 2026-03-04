// GameOverSubsystem.h
// 게임 오버 UI 관리 서브시스템
// 플레이어 사망/게임오버 시 호출하여 UI 표시

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameOverSubsystem.generated.h"

class UGameOverWidget;

UCLASS()
class UGameOverSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * 게임 오버 UI 표시
	 * 죽거나 게임 종료 조건 충족 시 호출
	 */
	UFUNCTION(BlueprintCallable, Category = "GameOver")
	void ShowGameOver();

	/** 게임 오버 UI 숨기기 (디버그/리셋용) */
	UFUNCTION(BlueprintCallable, Category = "GameOver")
	void HideGameOver();

	/** 게임 오버 상태인지 */
	UFUNCTION(BlueprintPure, Category = "GameOver")
	bool IsGameOver() const;

protected:

	UPROPERTY()
	TSubclassOf<UGameOverWidget> GameOverWidgetClass;

	UPROPERTY()
	UGameOverWidget* GameOverWidget;

	UGameOverWidget* GetOrCreateWidget();
};
