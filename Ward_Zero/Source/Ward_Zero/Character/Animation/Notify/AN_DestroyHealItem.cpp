#include "Character/Animation/Notify/AN_DestroyHealItem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

void UAN_DestroyHealItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(MeshComp->GetOwner()))
    {
        Player->DestroyHealItemVisual();
    }
}
