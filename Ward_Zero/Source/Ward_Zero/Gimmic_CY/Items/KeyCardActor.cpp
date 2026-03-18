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

	//todo: bIsActivated = SaveManager->CheckActivated(ActorID)
	bool bIsActivated = false;
	if (bIsActivated)
	{
		HiddenActor();
	}
}


void AKeyCardActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
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
	bCanInteract = false;
}


EInteractionType AKeyCardActor::GetInteractionType_Implementation() const
{
    return EInteractionType::Key;
}

