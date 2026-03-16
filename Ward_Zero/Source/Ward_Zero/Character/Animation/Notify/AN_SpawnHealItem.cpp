#include "Character/Animation/Notify/AN_SpawnHealItem.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

void UAN_SpawnHealItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(MeshComp->GetOwner()))
    {
        Player->SpawnHealItemVisual();
    }
}
