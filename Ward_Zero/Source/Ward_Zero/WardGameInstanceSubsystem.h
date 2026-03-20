// WardGameInstance.h
// 게임 인스턴스 서브시스템 — 세이브 데이터 + 옵션 값 보관
// PlayerController 접근 없이 어디서든 접근 가능

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI_KWJ/Save/WardSaveGame.h"
#include "WardGameInstanceSubsystem.generated.h"

UCLASS()
class WARD_ZERO_API UWardGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// ══════════════════════════════════════════
	//  PendingSaveData — 로드 대기 데이터
	// ══════════════════════════════════════════

	void SetPendingSaveData(UWardSaveGame* SaveData);

	UFUNCTION(BlueprintPure, Category = "Save")
	UWardSaveGame* GetPendingSaveData() const { return PendingSaveData; }

	void ClearPendingSaveData();

	// ══════════════════════════════════════════
	//  오브젝트 상태 (GUID 기반)
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetObjectState(const FGuid& SaveID, bool bActive, bool bCanInteract);

	UFUNCTION(BlueprintPure, Category = "Save")
	FObjectSaveData GetObjectState(const FGuid& SaveID) const;

	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasObjectState(const FGuid& SaveID) const;

	const TMap<FGuid, FObjectSaveData>& GetRuntimeObjectStates() const { return RuntimeObjectStates; }
	void SetRuntimeObjectStates(const TMap<FGuid, FObjectSaveData>& States);

	// ══════════════════════════════════════════
	//  옵션 값 (레벨 전환에도 유지)
	// ══════════════════════════════════════════

	/** 마스터 볼륨 (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Options|Audio")
	float MasterVolume = 1.0f;

	/** 효과음 볼륨 (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Options|Audio")
	float SFXVolume = 1.0f;

	/** 배경음 볼륨 (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Options|Audio")
	float BGMVolume = 1.0f;

	/** 감마 (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Options|Display")
	float Gamma = 0.5f;

	/** 옵션 값 적용 (볼륨/감마를 실제 시스템에 반영) */
	UFUNCTION(BlueprintCallable, Category = "Options")
	void ApplyOptions(UWorld* World);

private:

	UPROPERTY()
	UWardSaveGame* PendingSaveData = nullptr;

	TMap<FGuid, FObjectSaveData> RuntimeObjectStates;
};
