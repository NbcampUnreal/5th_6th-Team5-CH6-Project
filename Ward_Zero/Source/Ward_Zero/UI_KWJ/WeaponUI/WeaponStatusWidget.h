// Fill out your copyright notice in the Description page of Project Settings.
// WeaponStatusWidget.h
// 총기 상태 UI 위젯
// - 현재 장착 무기 이미지 + 탄약 수 표시
// - 무기 교체 시 하이라이트 연출 (크기 확대 후 복귀)
// - 비활성 무기 이미지는 완전히 숨김

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponStatusWidget.generated.h"

class UImage;
class UTextBlock;
class UCanvasPanel;

UCLASS(BlueprintType, Blueprintable)
class WARD_ZERO_API UWeaponStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  BindWidget
	// ══════════════════════════════════════════

	/** 권총 이미지 */
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Pistol;

	/** SMG 이미지 */
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_SMG;

	/** 탄약 텍스트 (예: "12 / 30") — 현재 탄약 / 전체 보유 탄약 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_CurrentAmmo;

	// ══════════════════════════════════════════
	//  공개 함수
	// ══════════════════════════════════════════

	/** 무기 교체 알림 */
	void OnWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn);

	/** 무기 집어넣기 알림 */
	void OnWeaponHolstered();

	/** 탄약 갱신 */
	void UpdateAmmoDisplay(int32 CurrentAmmo, int32 MaxCapacity, int32 ReserveAmmo);

	/** 표시/숨기기 */
	void SetWeaponUIVisible(bool bVisible);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

	// ── 상태 ──
	int32 ActiveWeaponIndex = 0; // 0=없음, 1=권총, 2=SMG
	bool bWeaponDrawn = false;

	// ── 이미지 스케일 보간 ──
	float PistolCurrentScale = 1.0f;
	float PistolTargetScale = 1.0f;
	float SMGCurrentScale = 1.0f;
	float SMGTargetScale = 1.0f;

	// ── 설정값 ──
	float ActiveScale = 1.0f;       // 평상시 활성 무기 스케일
	float HighlightScale = 1.3f;    // 교체 직후 하이라이트 스케일
	float ScaleInterpSpeed = 8.0f;

	float HighlightDuration = 1.5f; // 하이라이트 유지 시간(초)

	// ── 하이라이트 타이머 ──
	FTimerHandle HighlightTimerHandle;
	void EndHighlight();

	// ── 내부 함수 ──
	void ApplyVisibility();
	void ApplyImageScale(UImage* InImage, float Scale);
};