// WardGameInstance.cpp

#include "WardGameInstanceSubsystem.h"
#include "Engine/Blueprint.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Ward_Zero.h"

// ════════════════════════════════════════════════════════
//  초기화 — DataTable 자동 로드
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// DataTable 자동 로드
	DocumentDataTable = LoadObject<UWardDocumentDataTable>(
		nullptr,
		TEXT("/Game/UI/read/MyWardDocumentDataTable.MyWardDocumentDataTable")
	);

	// Blueprint DataAsset인 경우 _C 경로로 재시도
	if (!DocumentDataTable)
	{
		UObject* LoadedObj = StaticLoadObject(UObject::StaticClass(), nullptr,
			TEXT("/Game/UI/read/MyWardDocumentDataTable.MyWardDocumentDataTable"));
		DocumentDataTable = Cast<UWardDocumentDataTable>(LoadedObj);
	}

	// Blueprint CDO에서 가져오기
	if (!DocumentDataTable)
	{
		UClass* LoadedClass = LoadClass<UWardDocumentDataTable>(
			nullptr,
			TEXT("/Game/UI/read/MyWardDocumentDataTable.MyWardDocumentDataTable_C")
		);
		if (LoadedClass)
		{
			DocumentDataTable = Cast<UWardDocumentDataTable>(LoadedClass->GetDefaultObject());
		}
	}

	if (DocumentDataTable)
	{
		UE_LOG(LogWard_Zero, Log, TEXT("DocumentDataTable 로드 성공: %d개 항목"), DocumentDataTable->Entries.Num());
	}
	else
	{
		UE_LOG(LogWard_Zero, Warning, TEXT("DocumentDataTable 로드 실패!"));
	}
}

// ════════════════════════════════════════════════════════
//  PendingSaveData
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::SetPendingSaveData(UWardSaveGame* SaveData)
{
	PendingSaveData = SaveData;

	if (SaveData)
	{
		SetCurrentStage(SaveData->StageIndex);
	}
	RuntimeObjectStates.Empty();
}

void UWardGameInstanceSubsystem::ClearPendingSaveData()
{
	PendingSaveData = nullptr;
}

// ════════════════════════════════════════════════════════
//  오브젝트 상태 (GUID 기반)
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::SetObjectState(const FGuid& SaveID, bool bActive, bool bCanInteract)
{
	if (!SaveID.IsValid()) return;

	FObjectSaveData Data;
	Data.bActive = bActive;
	Data.bCanInteract = bCanInteract;
	RuntimeObjectStates.Add(SaveID, Data);
}

FObjectSaveData UWardGameInstanceSubsystem::GetObjectState(const FGuid& SaveID) const
{
	if (!SaveID.IsValid()) return FObjectSaveData();

	if (const FObjectSaveData* Data = RuntimeObjectStates.Find(SaveID))
	{
		return *Data;
	}

	if (PendingSaveData)
	{
		if (const FObjectSaveData* Data = PendingSaveData->ObjectStates.Find(SaveID))
		{
			return *Data;
		}
	}

	return FObjectSaveData();
}

bool UWardGameInstanceSubsystem::HasObjectState(const FGuid& SaveID) const
{
	if (!SaveID.IsValid()) return false;
	if (RuntimeObjectStates.Contains(SaveID)) return true;
	if (PendingSaveData && PendingSaveData->ObjectStates.Contains(SaveID)) return true;
	return false;
}

// ════════════════════════════════════════════════════════
//  스테이지 관리
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::SetCurrentStage(int32 InStage)
{
	// 역행 방지 — 현재보다 높은 스테이지만 저장
	if (InStage > CurrentStage)
	{
		CurrentStage = InStage;
		UE_LOG(LogWard_Zero, Log, TEXT("스테이지 갱신: %d"), CurrentStage);
	}
	else
	{
		UE_LOG(LogWard_Zero, Log, TEXT("스테이지 역행 방지: 요청=%d, 현재=%d → 무시"), InStage, CurrentStage);
	}
}
void UWardGameInstanceSubsystem::SetRuntimeObjectStates(const TMap<FGuid, FObjectSaveData>& States)
{
	RuntimeObjectStates = States;
	UE_LOG(LogWard_Zero, Log, TEXT("오브젝트 상태 복원: %d개"), RuntimeObjectStates.Num());
}

// ════════════════════════════════════════════════════════
//  옵션 적용
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::ApplyOptions(UWorld* World)
{
	if (!World) return;

	// 마스터 볼륨
	FAudioDeviceHandle AudioDevice = World->GetAudioDevice();
	if (AudioDevice.IsValid())
	{
		AudioDevice->SetTransientPrimaryVolume(MasterVolume);
	}

	// 감마
	if (GEngine)
	{
		float GammaValue = FMath::Lerp(1.5f, 3.5f, Gamma);
		GEngine->DisplayGamma = GammaValue;
	}

	UE_LOG(LogWard_Zero, Log, TEXT("옵션 적용: Master=%.0f%% SFX=%.0f%% BGM=%.0f%% Gamma=%.2f"),
		MasterVolume * 100.f, SFXVolume * 100.f, BGMVolume * 100.f, Gamma);
}

// ════════════════════════════════════════════════════════
//  문서/아이템 인덱스 관리
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::ActivateDocumentIndex(int32 DocIndex)
{
	if (DocIndex < 0) return;

	ActiveDocumentIndices.Add(DocIndex);
	UE_LOG(LogWard_Zero, Log, TEXT("문서 인덱스 활성화: %d (총 %d개)"), DocIndex, ActiveDocumentIndices.Num());
}

bool UWardGameInstanceSubsystem::IsDocumentIndexActive(int32 DocIndex) const
{
	return ActiveDocumentIndices.Contains(DocIndex);
}

bool UWardGameInstanceSubsystem::GetDocumentEntry(int32 DocIndex, FWardDocumentEntry& OutEntry) const
{
	if (!DocumentDataTable) return false;

	const FWardDocumentEntry* Found = DocumentDataTable->FindByIndex(DocIndex);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}

	return false;
}

// ════════════════════════════════════════════════════════
//  아이템 최초 습득 기록
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::MarkItemNotified(int32 ItemIndex)
{
	if (ItemIndex < 0) return;

	NotifiedItemIndices.Add(ItemIndex);
	UE_LOG(LogWard_Zero, Log, TEXT("아이템 알림 기록: %d (총 %d개)"), ItemIndex, NotifiedItemIndices.Num());
}

bool UWardGameInstanceSubsystem::IsItemNotified(int32 ItemIndex) const
{
	return NotifiedItemIndices.Contains(ItemIndex);
}

// ════════════════════════════════════════════════════════
//  보스 격파 관리
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::SetBossDefeated(FName BossID)
{
	if (BossID.IsNone()) return;

	DefeatedBosses.Add(BossID);
	UE_LOG(LogWard_Zero, Log, TEXT("보스 격파 등록: %s (총 %d마리)"), *BossID.ToString(), DefeatedBosses.Num());
}

bool UWardGameInstanceSubsystem::IsBossDefeated(FName BossID) const
{
	return !BossID.IsNone() && DefeatedBosses.Contains(BossID);
}

// ════════════════════════════════════════════════════════
//  새 게임 초기화
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::ResetForNewGame()
{
	RuntimeObjectStates.Empty();
	ActiveDocumentIndices.Empty();
	NotifiedItemIndices.Empty();
	DefeatedBosses.Empty();
	CurrentStage = 0;
	PendingSaveData = nullptr;

	UE_LOG(LogWard_Zero, Log, TEXT("새 게임 — 인스턴스 데이터 초기화 완료"));
}
