#include "KeyCardActor.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

AKeyCardActor::AKeyCardActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AKeyCardActor::BeginPlay()
{
	Super::BeginPlay();
}


void AKeyCardActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	Super::HandleInteraction_Implementation(Character);
	if (!bCanInteract)
		return;
	
	for (AActor* Actor : TargetActors)
	{
		IInteractionBase* InteractableActor = Cast<IInteractionBase>(Actor);
		if (InteractableActor)
		{
			InteractableActor->SetBCanInteract(true);
		}
	}
	SetBCanInteract(false);
}


EInteractionType AKeyCardActor::GetInteractionType_Implementation() const
{
    return EInteractionType::Key;
}

