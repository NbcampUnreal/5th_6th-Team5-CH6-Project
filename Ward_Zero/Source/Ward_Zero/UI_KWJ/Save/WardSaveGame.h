// WardSaveGame.h
// 세이브 파일에 저장되는 데이터 구조

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WardSaveGame.generated.h"

UCLASS()
class WARD_ZERO_API UWardSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UWardSaveGame();

	// ══════════════════════════════════════════
	//  플레이어 상태
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FVector PlayerLocation;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FRotator PlayerRotation;

	/** 체력 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Health")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Health")
	float MaxHealth;

	/** 스태미나 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Stamina")
	float CurrentStamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Stamina")
	float MaxStamina;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Stamina")
	bool bIsExhausted;

	// ══════════════════════════════════════════
	//  무기 & 탄약
	// ══════════════════════════════════════════

	/** 무기 장착 여부 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	bool bIsWeaponEquipped;

	/** 현재 탄약 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 CurrentAmmo;

	/** 탄창 최대 용량 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 MaxAmmoCapacity;

	// ══════════════════════════════════════════
	//  손전등
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData|FlashLight")
	bool bIsFlashLightOn;

	// ══════════════════════════════════════════
	//  월드 상태
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FName CurrentLevelName;

	// ══════════════════════════════════════════
	//  파트 진행도 — 각 파트 클리어 여부
	//  레벨 디자이너가 파트별 트리거를 만들면
	//  로드 시 이 값에 따라 트리거 on/off
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	bool bPart1Cleared = false;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	bool bPart2Cleared = false;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	bool bPart3Cleared = false;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	bool bPart4Cleared = false;

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	bool bPart5Cleared = false;

	// ══════════════════════════════════════════
	//  메타 정보
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FDateTime SaveDateTime;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	float PlayTimeSeconds;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FString DisplayName;

	// ══════════════════════════════════════════
	//  스크린샷
	// ══════════════════════════════════════════

	UPROPERTY()
	TArray<uint8> ScreenshotData;

	UPROPERTY()
	int32 ScreenshotWidth;

	UPROPERTY()
	int32 ScreenshotHeight;
};
