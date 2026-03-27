// WeaponUISubsystem.cpp

#include "UI_KWJ/WeaponUI/WeaponUISubsystem.h"
#include "UI_KWJ/WeaponUI/WeaponStatusWidget.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Weapon.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Ward_Zero.h"

void UWeaponUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogWard_Zero, Log, TEXT("WeaponUISubsystem 초기화 완료"));
}

void UWeaponUISubsystem::Deinitialize()
{
	if (WeaponWidget)
	{
		WeaponWidget->RemoveFromParent();
		WeaponWidget = nullptr;
	}
	Super::Deinitialize();
}

// ════════════════════════════════════════════════════════
//  StatusComp 델리게이트 바인딩
// ════════════════════════════════════════════════════════

void UWeaponUISubsystem::BindToStatusComponent(UPlayerStatusComponent* StatusComp)
{
	if (!StatusComp) return;

	// 이전 바인딩 해제 (ServerTravel 후 새 캐릭터에 재바인딩)
	if (BoundStatusComp && IsValid(BoundStatusComp))
	{
		BoundStatusComp->OnPistolAmmoChanged.RemoveDynamic(this, &UWeaponUISubsystem::OnPistolAmmoChanged);
		BoundStatusComp->OnSMGAmmoChanged.RemoveDynamic(this, &UWeaponUISubsystem::OnSMGAmmoChanged);
	}

	// 새 StatusComp에 바인딩
	StatusComp->OnPistolAmmoChanged.AddDynamic(this, &UWeaponUISubsystem::OnPistolAmmoChanged);
	StatusComp->OnSMGAmmoChanged.AddDynamic(this, &UWeaponUISubsystem::OnSMGAmmoChanged);

	BoundStatusComp = StatusComp;
	UE_LOG(LogWard_Zero, Log, TEXT("WeaponUI: StatusComp 탄약 델리게이트 바인딩 완료"));
}

void UWeaponUISubsystem::OnPistolAmmoChanged(int32 Current, int32 Max, int32 Reserve)
{
	if (WeaponIdx != 1) return;
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->UpdateAmmoDisplay(Current, Max, Reserve);
	}
}

void UWeaponUISubsystem::OnSMGAmmoChanged(int32 Current, int32 Max, int32 Reserve)
{
	if (WeaponIdx != 2) return;
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->UpdateAmmoDisplay(Current, Max, Reserve);
	}
}

// ════════════════════════════════════════════════════════
//  무기 교체 알림
// ════════════════════════════════════════════════════════

void UWeaponUISubsystem::NotifyWeaponChanged(int32 NewWeaponIndex, bool bIsDrawn)
{
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	WeaponIdx = NewWeaponIndex;
	if (Widget)
	{
		Widget->OnWeaponChanged(NewWeaponIndex, bIsDrawn);

		if (bIsDrawn)
		{
			Widget->SetWeaponUIVisible(true);

			// 무기 꺼낼 때 현재 탄약 즉시 표시
			APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
			if (PC)
			{
				APrototypeCharacter* Character = Cast<APrototypeCharacter>(PC->GetPawn());
				if (Character)
				{
					UPlayerCombatComponent* Combat = Character->FindComponentByClass<UPlayerCombatComponent>();
					if (Combat)
					{
						AWeapon* Weapon = Combat->GetEquippedWeapon();
						if (Weapon)
						{
							Widget->UpdateAmmoDisplay(
								Weapon->GetCurrentAmmo(),
								Weapon->GetMaxCapacity(),
								Weapon->GetReserveAmmo()
							);
						}
					}
				}
			}
		}
		else
		{
			Widget->SetWeaponUIVisible(false);
		}
	}
}

void UWeaponUISubsystem::NotifyWeaponHolstered()
{
	UWeaponStatusWidget* Widget = GetOrCreateWidget();
	if (Widget)
	{
		Widget->OnWeaponHolstered();
		Widget->SetWeaponUIVisible(false);
	}
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
			WeaponWidget->SetAnchorsInViewport(FAnchors(1.0f, 1.0f));
			WeaponWidget->SetAlignmentInViewport(FVector2D(1.0f, 1.0f));
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
		WeaponWidget->SetAnchorsInViewport(FAnchors(1.0f, 1.0f));
		WeaponWidget->SetAlignmentInViewport(FVector2D(1.0f, 1.0f));
		WeaponWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	return WeaponWidget;
}
