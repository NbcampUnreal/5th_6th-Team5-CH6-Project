// WardGameInstance.cpp

#include "WardGameInstance.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Ward_Zero.h"

// ════════════════════════════════════════════════════════
//  PendingSaveData
// ════════════════════════════════════════════════════════

void UWardGameInstance::SetPendingSaveData(UWardSaveGame* SaveData)
{
	PendingSaveData = SaveData;
}

void UWardGameInstance::ClearPendingSaveData()
{
	PendingSaveData = nullptr;
}

// ════════════════════════════════════════════════════════
//  오브젝트 상태 (GUID 기반)
// ════════════════════════════════════════════════════════

void UWardGameInstance::SetObjectState(const FGuid& SaveID, bool bActive, bool bCanInteract)
{
	if (!SaveID.IsValid()) return;

	FObjectSaveData Data;
	Data.bActive = bActive;
	Data.bCanInteract = bCanInteract;
	RuntimeObjectStates.Add(SaveID, Data);
}

FObjectSaveData UWardGameInstance::GetObjectState(const FGuid& SaveID) const
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

bool UWardGameInstance::HasObjectState(const FGuid& SaveID) const
{
	if (!SaveID.IsValid()) return false;
	if (RuntimeObjectStates.Contains(SaveID)) return true;
	if (PendingSaveData && PendingSaveData->ObjectStates.Contains(SaveID)) return true;
	return false;
}

void UWardGameInstance::SetRuntimeObjectStates(const TMap<FGuid, FObjectSaveData>& States)
{
	RuntimeObjectStates = States;
	UE_LOG(LogWard_Zero, Log, TEXT("오브젝트 상태 복원: %d개"), RuntimeObjectStates.Num());
}

// ════════════════════════════════════════════════════════
//  옵션 적용
// ════════════════════════════════════════════════════════

void UWardGameInstance::ApplyOptions(UWorld* World)
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
