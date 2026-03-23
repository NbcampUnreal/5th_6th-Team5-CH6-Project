// WardSaveGame.cpp

#include "UI_KWJ/Save/WardSaveGame.h"

UWardSaveGame::UWardSaveGame()
{
	PlayerLocation = FVector::ZeroVector;
	PlayerRotation = FRotator::ZeroRotator;
	CurrentHealth = 100.0f;
	MaxHealth = 100.0f;
	CurrentStamina = 100.0f;
	MaxStamina = 100.0f;
	bIsExhausted = false;
	bIsWeaponEquipped = false;
	CurrentAmmo = 0;
	MaxAmmoCapacity = 12;
	ReserveAmmo = 0;
	bIsFlashLightOn = false;
	CurrentLevelName = NAME_None;
	PlayTimeSeconds = 0.0f;
	ScreenshotWidth = 0;
	ScreenshotHeight = 0;

	// 파트 진행도
	CurrentPart = 0;
	CurrentSubPart = 0;

	// 오브젝트 상태 (TMap은 빈 상태로 초기화됨)
}
