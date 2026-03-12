// LoadingScreenSubsystem.h
// 로딩 화면 — Movie Player 시스템 사용
// ServerTravel/OpenLevel 중에도 위젯이 살아남음

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LoadingScreenSubsystem.generated.h"

class ULoadingWidget;

UCLASS()
class WARD_ZERO_API ULoadingScreenSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** 로딩 화면 표시 — ServerTravel/OpenLevel 직전에 호출 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoading(const FText& Message = INVTEXT("Loading..."));

	/** 로딩 화면 수동 숨기기 (같은 레벨 로드 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HideLoading();

	UFUNCTION(BlueprintPure, Category = "Loading")
	bool IsLoading() const;

private:

	bool bIsLoading = false;
};
