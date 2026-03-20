#include "Character/Components/Status/PlayerStatusComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Gimmic_CY/Items/HealItemActor.h"
#include "Components/WidgetComponent.h"

UPlayerStatusComponent::UPlayerStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickInterval(0.2f);

	CurrHealth = MaxHealth;
	bIsDead = false;

	CurrStamina = MaxStamina;
	bIsExhausted = false;
}

void UPlayerStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead) return;

	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	// 캐릭터의 GetIsRunning() 상태에 따라 스테미나 소모/회복
	if (Player->GetIsRunning())
	{
		// BeginPlay에서 DataAsset 값이 세팅된 StaminaDrainRate 사용
		CurrStamina = FMath::Clamp(CurrStamina - (StaminaDrainRate * DeltaTime), 0.0f, MaxStamina);
		if (CurrStamina <= 0.0f) bIsExhausted = true;
	}
	else
	{
		CurrStamina = FMath::Clamp(CurrStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);
		if (bIsExhausted && CurrStamina >= MinStaminaToSprint) bIsExhausted = false;
	}
}

float UPlayerStatusComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDead) return 0.0f;

	float ActualDamage = FMath::Min(CurrHealth, DamageAmount);
	CurrHealth -= ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("[Player] HP: %.1f / %.1f"), CurrHealth, MaxHealth);

	if (CurrHealth <= 0.0f)
	{
		CurrHealth = 0.0f;
		bIsDead = true;
		OnPlayerDied.Broadcast(); //캐릭터 Die 알림 
	}
	OnHealthChanged.Broadcast(CurrHealth, MaxHealth);

	return ActualDamage;
}

void UPlayerStatusComponent::UpdateAmmoUI(int32 WeaponIndex, int32 Current, int32 Max, int32 Reserve)
{
	if (WeaponIndex == 1) // Pistol
	{
		OnPistolAmmoChanged.Broadcast(Current, Max, Reserve);
	}
	else if (WeaponIndex == 2) // SMG
	{
		OnSMGAmmoChanged.Broadcast(Current, Max, Reserve);
	}
}

void UPlayerStatusComponent::ReviveStatus(float HealthRatio)
{
	bIsDead = false;
	CurrHealth = MaxHealth * HealthRatio;

	// 체력 UI 갱신 
	OnHealthChanged.Broadcast(CurrHealth, MaxHealth);
}

void UPlayerStatusComponent::Heal(float Amount)
{
	if (bIsDead) return;

	CurrHealth = FMath::Clamp(CurrHealth + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrHealth, MaxHealth);

	UE_LOG(LogTemp, Log, TEXT("Healed! Current HP: %.1f"), CurrHealth);
}

bool UPlayerStatusComponent::AddHealingItem(int32 Amount)
{
	if (HealingItemCount >= MaxHealingItemCount) return false;

	HealingItemCount = FMath::Clamp(HealingItemCount + Amount, 0, MaxHealingItemCount);
	return true;
}

void UPlayerStatusComponent::SpawnHealItemVisual(USkeletalMeshComponent* Mesh)
{
	if (!HealItemClass || !Mesh) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();

	CurrHealItem = GetWorld()->SpawnActor<AActor>(HealItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrHealItem)
	{
		CurrHealItem->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HealItemSocket"));
		CurrHealItem->SetActorRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

		AHealItemActor* HealProp = Cast<AHealItemActor>(CurrHealItem);
		if (HealProp)
		{
			if (HealProp->MarkerPillar) HealProp->MarkerPillar->SetHiddenInGame(true);
			if (HealProp->InteractWidget) HealProp->InteractWidget->SetVisibility(false);
		}
	}
}

void UPlayerStatusComponent::DestroyHealItemVisual()
{
	if (CurrHealItem)
	{
		CurrHealItem->SetActorHiddenInGame(true);
		CurrHealItem->SetActorEnableCollision(false);
		CurrHealItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurrHealItem = nullptr;
	}
}

void UPlayerStatusComponent::PopHealItemCap()
{
	if (CurrHealItem)
	{
		AHealItemActor* HealProp = Cast<AHealItemActor>(CurrHealItem);
		if (HealProp && HealProp->CapMesh)
		{
			HealProp->CapMesh->SetHiddenInGame(true);
		}
	}
}