// LoadingScreenSubsystem.h
// 로딩 화면 — 맵 전환, 세이브 로드 시 표시

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

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 로딩 화면 표시 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoading(const FText& Message = INVTEXT("Loading..."));

	/** 로딩 화면 숨기기 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HideLoading();

	UFUNCTION(BlueprintPure, Category = "Loading")
	bool IsLoading() const;

private:

	UPROPERTY()
	TSubclassOf<ULoadingWidget> LoadingWidgetClass;

	UPROPERTY()
	ULoadingWidget* LoadingWidget;

	ULoadingWidget* GetOrCreateWidget();

	/** 레벨 로드 완료 시 자동 숨기기 */
	void OnLevelLoaded(UWorld* LoadedWorld);
	FDelegateHandle LevelLoadedHandle;
};
