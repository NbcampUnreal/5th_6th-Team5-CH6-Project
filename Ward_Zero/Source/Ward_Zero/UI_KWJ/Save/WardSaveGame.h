// WardSaveGame.h
// 세이브 파일에 저장되는 데이터 구조

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WardSaveGame.generated.h"

/** 오브젝트 저장 데이터 — GUID와 함께 저장 */
USTRUCT(BlueprintType)
struct FObjectSaveData
{
	GENERATED_BODY()

	/** 활성화 여부 (레버 당김, 벽 파괴, 아이템 습득 등) */
	UPROPERTY(VisibleAnywhere)
	bool bActive = false;

	/** 상호작용 가능 여부 */
	UPROPERTY(VisibleAnywhere)
	bool bCanInteract = true;
};

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

	/** 권총 — 현재 탄약 (탄창 안) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 CurrentAmmo;

	/** 권총 — 탄창 최대 용량 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 MaxAmmoCapacity;

	/** 권총 — 예비 탄약 (탄창 밖) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 ReserveAmmo;

	/** SMG — 현재 탄약 (탄창 안) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 SMGCurrentAmmo;

	/** SMG — 탄창 최대 용량 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 SMGMaxAmmoCapacity;

	/** SMG — 예비 탄약 (탄창 밖) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Weapon")
	int32 SMGReserveAmmo;

	// ══════════════════════════════════════════
	//  수집한 서류 (인덱스 20+)
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Documents")
	TSet<int32> CollectedDocumentIndices;

	// ══════════════════════════════════════════
	//  아이템 최초 습득 기록 (인덱스 0~19)
	//  알림 위젯 중복 방지용
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Items")
	TSet<int32> NotifiedItemIndices;

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

	/** 스테이지 인덱스 (0=지하1층, 1=1층, 2=2층) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	int32 StageIndex = 0;

	/** 격파된 보스 ID 목록 (예: "Huton") */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Progress")
	TSet<FName> DefeatedBosses;

	// ══════════════════════════════════════════
	//  오브젝트 상태 (GUID 기반)
	//  각 액터가 GUID + Active/Interaction 상태를 저장
	//  Active: 활성화 여부 (레버 당김, 벽 파괴 등)
	//  CanInteract: 상호작용 가능 여부
	// ══════════════════════════════════════════

	UPROPERTY(VisibleAnywhere, Category = "SaveData|Objects")
	TMap<FGuid, FObjectSaveData> ObjectStates;

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
