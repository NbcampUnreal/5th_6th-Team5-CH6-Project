// WardGameInstance.cpp

#include "WardGameInstanceSubsystem.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Ward_Zero.h"

// ════════════════════════════════════════════════════════
//  PendingSaveData
// ════════════════════════════════════════════════════════

void UWardGameInstanceSubsystem::SetPendingSaveData(UWardSaveGame* SaveData)
{
	PendingSaveData = SaveData;
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
