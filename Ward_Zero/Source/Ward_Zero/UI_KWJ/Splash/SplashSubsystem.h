// SplashSubsystem.h
// 게임 시작 로고 스플래시 관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "SplashSubsystem.generated.h"

class USplashWidget;

UCLASS()
class WARD_ZERO_API USplashSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * 스플래시 표시 → 완료 후 OnComplete 호출
	 * 메인메뉴 레벨의 BeginPlay에서 호출
	 * 스플래시 완료 후 메인메뉴 표시
	 */
	UFUNCTION(BlueprintCallable, Category = "Splash")
	void ShowSplash();

	/** 스플래시가 표시 중인지 */
	UFUNCTION(BlueprintPure, Category = "Splash")
	bool IsSplashPlaying() const;

private:

	UPROPERTY()
	USplashWidget* SplashWidget;

	UFUNCTION()
	void OnSplashFinished();

	/** 이미 한 번 표시했는지 (세션당 1회) */
	bool bHasShownSplash = false;
};
