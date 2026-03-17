#include "KeyCardActor.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

AKeyCardActor::AKeyCardActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

//void AKeyCardActor::OnIneracted_Implementation(APrototypeCharacter* Character)
//{
//	Super::OnIneracted_Implementation(Character);
//}
//
//void AKeyCardActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
//{
//	if (!Character || !bCanInteract) return;
//
//	// 플레이어에게 카드키 지급
//	Character->GiveKeyCard();
//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "Key Card Acquired");
//
//	// 카드키 제거
//	Destroy();
//}

EInteractionType AKeyCardActor::GetInteractionType_Implementation() const
{
    return EInteractionType::Key;
}
