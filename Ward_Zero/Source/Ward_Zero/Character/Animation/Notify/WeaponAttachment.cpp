#include "Character/Animation/Notify/WeaponAttachment.h"
#include "WeaponAttachment.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/PlayerCombatComponent.h"

void UWeaponAttachment::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (APrototypeCharacter* Character = Cast<APrototypeCharacter>(MeshComp->GetOwner()))
	{
		if (UPlayerCombatComponent* Combat = Character->FindComponentByClass<UPlayerCombatComponent>())
		{
			Combat->HandleWeaponAttachment(bAttachToHand);
		}
	}
}
