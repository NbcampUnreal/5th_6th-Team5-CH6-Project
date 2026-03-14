#include "KeyCardActor.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

AKeyCardActor::AKeyCardActor()
{
	PrimaryActorTick.bCanEverTick = false;

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	RootComponent = KeyMesh;

	KeyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KeyMesh->SetCollisionResponseToAllChannels(ECR_Block);
	KeyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void AKeyCardActor::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void AKeyCardActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character || !bCanInteract) return;

	// 플레이어에게 카드키 지급
	Character->GiveKeyCard();
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "Key Card Acquired");

	// 카드키 제거
	Destroy();
}

EInteractionType AKeyCardActor::GetInteractionType_Implementation() const
{
    return EInteractionType::Key;
}

bool AKeyCardActor::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	return bCanInteract;
}

bool AKeyCardActor::GetBCanInteract() const
{
	return bCanInteract;
}

void AKeyCardActor::HiddenActor()
{
}
