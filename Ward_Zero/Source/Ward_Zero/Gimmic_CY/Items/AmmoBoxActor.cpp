#include "Gimmic_CY/Items/AmmoBoxActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Weapon.h"
#include "Components/WidgetComponent.h"

AAmmoBoxActor::AAmmoBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;
	InHandRotator = FRotator(0.f, 90.f, 0.f);
}

void AAmmoBoxActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAmmoBoxActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;

	Super::HandleInteraction_Implementation(Character);
	
	if (!bCanInteract)
		return;
	UPlayerCombatComponent* CombatComp = Character->FindComponentByClass<UPlayerCombatComponent>();
	if (CombatComp)
	{
		AWeapon* TargetWeapon = nullptr;

		if (TargetWeaponIndex == 1) TargetWeapon = CombatComp->PistolWeapon;
		else if (TargetWeaponIndex == 2) TargetWeapon = CombatComp->SMGWeapon;

		if (TargetWeapon)
		{
			TargetWeapon->AddAmmo(AmmoAmount);

			UE_LOG(LogTemp, Warning, TEXT("Looted %d Ammo for Weapon %d"), AmmoAmount, TargetWeaponIndex);
			
			
		}
	}
	SetBCanInteract(false);
}

EInteractionType AAmmoBoxActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Ammo;
}

