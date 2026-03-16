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
	//  파트 진행도 (몬스터 트리거 관리)
	//  이전 파트의 몬스터는 전부 비활성화
	// ══════════════════════════════════════════

	/** 현재 파트 번호 (0=시작, 1=지하1층, 2=2층보스, 3=1층, 4=촉수) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	int32 CurrentPart = 0;

	/** 현재 파트 내 서브파트 (좀비 웨이브 진행도) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	int32 CurrentSubPart = 0;

	// ══════════════════════════════════════════
	//  오브젝트 상태 (ID 기반)
	//  맵에 배치된 오브젝트에 고유 ID를 부여하고
	//  해당 오브젝트의 상태를 bool로 저장
	//
	//  ID 접두어 규칙 (팀 협의에 따라 변경 가능):
	//    "lv_"  = 레버 (예: "lv_01", "lv_02")
	//    "hl_"  = 힐 아이템 (예: "hl_01", "hl_02")
	//    "dr_"  = 잠긴 문 (예: "dr_01")
	//    "st_"  = 셔터 (예: "st_01")
	//
	//  저장 시: 맵에서 ID가 있는 오브젝트를 순회 → 상태 수집
	//  로드 시: 맵에서 ID 매칭 → 저장된 상태 적용
	//           매칭 안 되면 스킵 (맵에 없는 오브젝트)
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Objects")
	TMap<FString, bool> SavedObjectStates;

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
