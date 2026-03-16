#include "Character/Animation/Notify/AN_PopHealCap.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

void UAN_PopHealCap::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(MeshComp->GetOwner()))
    {
        Player->PopHealItemCap();
    }
}
