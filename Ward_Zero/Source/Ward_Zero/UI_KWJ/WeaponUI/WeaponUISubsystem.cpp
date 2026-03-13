// WeaponUISubsystem.cpp

#include "UI_KWJ/WeaponUI/WeaponUISubsystem.h"
#include "UI_KWJ/WeaponUI/WeaponStatusWidget.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Weapon.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Ward_Zero.h"

void UWeaponUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogWard_Zero, Log, TEXT("WeaponUISubsystem 초기화 완료"));
}

void UWeaponUISubsystem::Deinitialize()
{
	StopAmmoPolling();

	if (WeaponWidget)
	{
		WeaponWidget->RemoveFromParent();
		WeaponWidget = nullptr;
	}
	Super::Deinitialize();
}

// ════════════════════════════════════════════════════════
//  탄약 폴링 (캐릭터 Tick 대체 — 10Hz)
// ════════════════════════════════════════════════════════

void UWeaponUISubsystem::StartAmmoPolling()
{
	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(AmmoUpdateTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				AmmoUpdateTimerHandle, this,
				&UWeaponUISubsystem::PollAmmoUpdate,
				0.1f, true); // 10Hz
		}
	}
}

void UWeaponUISubsystem::StopAmmoPolling()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AmmoUpdateTimerHandle);
	}
}

void UWeaponUISubsystem::PollAmmoUpdate()
{
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (!Widget) return;

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return;

	APrototypeCharacter* Character = Cast<APrototypeCharacter>(PC->GetPawn());
	if (!Character) return;

	UPlayerCombatComponent* Combat = Character->FindComponentByClass<UPlayerCombatComponent>();
	if (!Combat) return;

	AWeapon* Weapon = Combat->GetEquippedWeapon();
	if (Weapon && Combat->IsWeaponDrawn())
	{
		Widget->SetWeaponUIVisible(true);
		Widget->UpdateAmmoDisplay(
			Weapon->GetCurrentAmmo(),
			Weapon->GetMaxCapacity(),
			Weapon->GetReserveAmmo()
		);
	}
	else
	{
		Widget->SetWeaponUIVisible(false);
		StopAmmoPolling();
	}
}

// ════════════════════════════════════════════════════════
//  무기 교체 알림
// ════════════════════════════════════════════════════════

void UWeaponUISubsystem::NotifyWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn)
{
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->OnWeaponChanged(NewWeaponIndex, bIsDrawn);
	}

	if (bIsDrawn)
	{
		StartAmmoPolling();
	}
	else
	{
		StopAmmoPolling();
	}
}

void UWeaponUISubsystem::NotifyWeaponHolstered()
{
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->OnWeaponHolstered();
	}
	StopAmmoPolling();
}

void UWeaponUISubsystem::SetWeaponUIVisible(bool bVisible)
{
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->SetWeaponUIVisible(bVisible);
	}
}

// ════════════════════════════════════════════════════════
//  위젯 생성
// ════════════════════════════════════════════════════════

UWeaponStatusWidget* UWeaponUISubsystem::GetOrCreateWidget()
{
	if (IsValid(WeaponWidget))
	{
		if (!WeaponWidget->IsInViewport())
		{
			WeaponWidget->AddToViewport(50);
			WeaponWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return WeaponWidget;
	}

	WeaponWidget = nullptr;

	if (!WidgetClass)
	{
		WidgetClass = LoadClass<UWeaponStatusWidget>(
			nullptr,
			TEXT("/Game/UI/WeaponUI/WBP_WeaponStatus.WBP_WeaponStatus_C")
		);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogWard_Zero, Error, TEXT("WBP_WeaponStatus를 찾을 수 없습니다!"));
		return nullptr;
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	WeaponWidget = CreateWidget<UWeaponStatusWidget>(PC, WidgetClass);
	if (WeaponWidget)
	{
		WeaponWidget->AddToViewport(50);
		WeaponWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return WeaponWidget;
}
