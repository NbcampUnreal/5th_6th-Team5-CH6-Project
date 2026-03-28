// WardGameInstance.h
// 게임 인스턴스 서브시스템 — 세이브 데이터 + 옵션 값 보관
// PlayerController 접근 없이 어디서든 접근 가능

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI_KWJ/Save/WardSaveGame.h"
#include "UI_KWJ/Reading/WardDocumentDataTable.h"
#include "WardGameInstanceSubsystem.generated.h"

UCLASS()
class WARD_ZERO_API UWardGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

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

	// ══════════════════════════════════════════
	//  문서/아이템 인덱스 관리
	//  액터가 인덱스로 알림 → 인스턴스에 등록
	//  UI에서 인스턴스 확인 → 활성화된 인덱스만 표시
	// ══════════════════════════════════════════

	/** 데이터 테이블 (에디터에서 할당 또는 코드에서 로드) */
	UPROPERTY(BlueprintReadWrite, Category = "Document")
	UWardDocumentDataTable* DocumentDataTable = nullptr;

	/** 문서/아이템 활성화 등록 (액터가 호출) */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void ActivateDocumentIndex(int32 DocIndex);

	/** 해당 인덱스가 활성화되었는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Document")
	bool IsDocumentIndexActive(int32 DocIndex) const;

	/** 활성화된 인덱스 목록 가져오기 (UI에서 사용) */
	UFUNCTION(BlueprintPure, Category = "Document")
	const TSet<int32>& GetActiveDocumentIndices() const { return ActiveDocumentIndices; }

	/** 인덱스로 데이터 조회 */
	UFUNCTION(BlueprintPure, Category = "Document")
	bool GetDocumentEntry(int32 DocIndex, FWardDocumentEntry& OutEntry) const;

	// ══════════════════════════════════════════
	//  스테이지 관리
	//  층별 전체 프리셋 (0=지하1층, 1=1층, 2=2층)
	//  세이브 액터가 저장 시 스테이지 인덱스 전달
	//  각 오브젝트는 자기 스테이지와 비교해 활성화 여부 판단
	// ══════════════════════════════════════════

	/** 현재 스테이지 설정 (Max 비교 — 역행 방지) */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void SetCurrentStage(int32 InStage);

	/** 현재 스테이지 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Stage")
	int32 GetCurrentStage() const { return CurrentStage; }

	// ══════════════════════════════════════════
	//  보스 격파 관리
	// ══════════════════════════════════════════

	/** 보스 격파 등록 (보스 사망 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void SetBossDefeated(FName BossID);

	/** 보스가 이미 격파되었는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsBossDefeated(FName BossID) const;

	const TSet<FName>& GetDefeatedBosses() const { return DefeatedBosses; }
	void SetDefeatedBosses(const TSet<FName>& Bosses) { DefeatedBosses = Bosses; }

	/** 새 게임 시작 시 모든 데이터 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void ResetForNewGame();

private:

	UPROPERTY()
	UWardSaveGame* PendingSaveData = nullptr;

	TMap<FGuid, FObjectSaveData> RuntimeObjectStates;

	/** 활성화된 문서/아이템 인덱스 세트 */
	TSet<int32> ActiveDocumentIndices;

	/** 현재 스테이지 (역행 방지 — Max로만 갱신) */
	int32 CurrentStage = 0;

	/** 런타임 보스 격파 목록 */
	TSet<FName> DefeatedBosses;
};
