// HealthVignetteWidget.h
// 체력에 따라 피 비네팅 효과를 표시하는 HUD 위젯
// - 체력 100%: 완전 투명
// - 체력 감소: 비네팅 점점 선명
// - 바이오하자드 스타일

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthVignetteWidget.generated.h"

class UImage;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UHealthVignetteWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 피 비네팅 이미지 (가장자리 핏빛 오버레이) */
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_BloodVignette;

	// ══════════════════════════════════════════
	//  공개 함수
	// ══════════════════════════════════════════

	/**
	 * 체력 비율에 따라 비네팅 투명도 갱신
	 * @param HealthPercent  0.0 (사망) ~ 1.0 (만피)
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void UpdateVignette(float HealthPercent);

	/** PlayerStatusComponent의 OnHealthChanged에 바인딩할 콜백 */
	UFUNCTION()
	void OnHealthChanged(float CurrentHP, float MaxHP);

	// ══════════════════════════════════════════
	//  설정값
	// ══════════════════════════════════════════

	/** 비네팅이 보이기 시작하는 체력 비율 (이 이하부터 표시) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
	float VignetteStartThreshold = 0.75f;

	/** 비네팅 최대 투명도 (사망 직전) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
	float MaxVignetteOpacity = 0.85f;

	/** 비네팅 전환 보간 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
	float InterpSpeed = 5.0f;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

	/** 목표 투명도 (보간 대상) */
	float TargetOpacity = 0.0f;

	/** 현재 투명도 */
	float CurrentOpacity = 0.0f;
};
