#include "Character/Animation/Notify/ReloadFinish.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Weapon/Weapon.h"

void UReloadFinish::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (APrototypeCharacter* Character = Cast<APrototypeCharacter>(MeshComp->GetOwner()))
    {
        if (AWeapon* Weapon = Character->GetEquippedWeapon())
        {
            Weapon->FinishReload();
        }
    }
}
